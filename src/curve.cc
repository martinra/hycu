/*============================================================================

    (C) 2016 Martin Westerholt-Raum

    This file is part of HyCu.

    HyCu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    HyCu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HyCu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

===============================================================================*/


#include <algorithm>
#include <cmath>
#include <flint/fq_nmod.h>
#include <flint/fq_nmod_poly.h>
#include <flint/fq_zech.h>
#include <flint/fmpz.h>
#include <flint/nmod_poly.h>
#include <map>
#include <memory>
#include <numeric>
#include <iostream>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>

#ifdef TIMING
#include <chrono>
#endif

#include "curve.hh"
#include "opencl/interface.hh"
#include "reduction_table.hh"


using namespace std;


ostream&
operator<<(
    ostream & stream,
    const Curve & curve
    )
{
  stream << "Y^2 = ";
  for (int ix=curve.poly_coeff_exponents.size()-1; ix>=0; --ix) {
    auto coeff_str = fq_nmod_get_str_pretty(curve.table->at(curve.poly_coeff_exponents[ix]), curve.table->fq_ctx);
    stream << "(" << coeff_str << ")*X^" << ix;
    flint_free(coeff_str);
    if (ix != 0) stream << " + ";
  }

  stream << "  /  " << "F_" << curve.table->prime_power;

  return stream;
}

Curve::
Curve(
    shared_ptr<FqElementTable> table,
    const vector<unsigned int> poly_coeff_exponents
    ) :
    table( table )
{
  this->poly_coeff_exponents = move(poly_coeff_exponents);

  while ( this->poly_coeff_exponents.back() == this->table->zero_index() )
    this->poly_coeff_exponents.pop_back();
}

unsigned int
Curve::
genus()
  const
{
  if (this->degree() % 2 == 0)
    return (this->degree() - 2) / 2;
  else
    return (this->degree() - 1) / 2;
}

vector<unsigned int>
Curve::
rhs_support()
  const
{
  auto zero_index = this->table->zero_index();
  vector<unsigned int> support;

  for ( size_t ix=0; ix<this->poly_coeff_exponents.size(); ++ix )
    if ( this->poly_coeff_exponents[ix] != zero_index )
      support.push_back(ix);

  return support;
}

bool
Curve::
has_squarefree_rhs()
{
  if ( this->table->is_prime_field() ) {
    auto poly = this->rhs_nmod_polynomial();
    bool is_squarefree = nmod_poly_is_squarefree(&poly);
    nmod_poly_clear(&poly);

    return is_squarefree;
  }
  else {
    auto poly = this->rhs_polynomial();
    bool is_squarefree = fq_nmod_poly_is_squarefree(&poly, this->table->fq_ctx);
    fq_nmod_poly_clear(&poly, this->table->fq_ctx);

    return is_squarefree;
  }
}

vector<unsigned int>
Curve::
convert_poly_coeff_exponents(
    const ReductionTable & table
    )
{
  if ( this->table->prime != table.prime ) {
    cerr << "Curve.convert_poly_coeff_exponents: Can only convert to same prime" << endl;
    throw;
  }

  if ( table.prime_exponent % this->table->prime_exponent != 0  ) {
    cerr << "Curve.convert_poly_coeff_exponents: Can not convert to prime exponent "
         << "that does not divide the one of the curve" << endl;
    throw;
  }
  else if ( this->table->prime_exponent == table.prime_exponent ) {
    return this->poly_coeff_exponents;
  }

  unsigned int prime_power_pred = this->table->prime_power_pred;
  unsigned int exponent_factor = table.prime_power_pred / this->table->prime_power_pred;

  vector<unsigned int> converted;
  converted.reserve(this->poly_coeff_exponents.size());
  for ( unsigned int c : this->poly_coeff_exponents )
    converted.push_back(c != prime_power_pred ? exponent_factor * c
                                              : table.prime_power_pred );

  return converted;
}

void
Curve::
count(
    ReductionTable & reduction_table
  )
{
  // todo: Use error checking for all calls. Some implementations don't seem to support try/catch, but doublecheck this.

  if ( reduction_table.prime != this->table->prime ) {
    cerr << "Curve.count: primes of curve and table must coincide: "
         << reduction_table.prime << " " << this->table->prime << endl;
    throw;
  }
  unsigned int prime_exponent = reduction_table.prime_exponent;

  if ( this->nmb_points.find( prime_exponent ) != this->nmb_points.end() )
    return;
  this->nmb_points[prime_exponent] = make_tuple(0,0);


  // this also checks that the prime exponent is divisible by the one of the curve
  const vector<unsigned int> poly_coeff_exponents = this->convert_poly_coeff_exponents(reduction_table);

  // ponts x != 0, infty
  if ( reduction_table.is_opencl_enabled() )
    this->count_opencl(reduction_table, poly_coeff_exponents);
  else
    this->count_cpu(reduction_table, poly_coeff_exponents);


  // point x = 0
  // if constant coefficient is zero
  if (poly_coeff_exponents.front() == reduction_table.prime_power_pred)
    get<1>(this->nmb_points[prime_exponent]) += 1;
  // if constant coefficient is even power of generator
  else if (!(poly_coeff_exponents.front() & 1))
    get<0>(this->nmb_points[prime_exponent]) += 2;


  // point x = infty
  // if poly_coeffs ends with zero entry
  if ( this->degree() < 2*this->genus() + 2 )
    get<1>(this->nmb_points[prime_exponent]) += 1;
  // if leading coefficient is even power of generator
  else if (!(poly_coeff_exponents.back() & 1))
    get<0>(this->nmb_points[prime_exponent]) += 2;
}

void
Curve::
count_opencl(
    ReductionTable & reduction_table,
    const vector<unsigned int> & poly_coeff_exponents
    )
{
#ifndef WITH_OPENCL
  cerr << "Curve::count_opencl: compiled without OpenCL support" << endl;
  throw;
#else // WITH_OPENCL

#ifdef TIMING
  chrono::steady_clock::time_point start;
  start = chrono::steady_clock::now();
#endif // TIMING
  reduction_table.kernel_evaluation(this->degree())->enqueue(poly_coeff_exponents);
#ifdef TIMING
    cerr << "  TIMING: counting opencl evaluation" << endl
         << "    "
         << chrono::duration<double, milli>(chrono::steady_clock::now() - start).count()
         << " ms" << endl;
    start = chrono::steady_clock::now();
#endif // TIMING
  reduction_table.kernel_reduction()->reduce(this->nmb_points);
#ifdef TIMING
    cerr << "  TIMING: counting opencl reduction" << endl
         << "    "
         << chrono::duration<double, milli>(chrono::steady_clock::now() - start).count()
         << " ms" << endl;
#endif // TIMING

#endif // WITH_OPENCL
}

void
Curve::
count_cpu(
    const ReductionTable & reduction_table,
    const vector<unsigned int> & poly_coeff_exponents
    )
{
  unsigned int prime_exponent = reduction_table.prime_exponent;
  unsigned int prime_power_pred = reduction_table.prime_power_pred;

  const auto & exponent_reduction_table = *reduction_table.exponent_reduction_table;
  const auto & incrementation_table = *reduction_table.incrementation_table;

  unsigned int poly_size = poly_coeff_exponents.size();


  for ( unsigned int x = 1; x <= prime_power_pred; ++x ) {
    unsigned int f = poly_coeff_exponents[0];
    for ( unsigned int dx=1, xpw=x; dx < poly_size; ++dx, xpw+=x ) {
      xpw = exponent_reduction_table[xpw];
      if ( poly_coeff_exponents[dx] != prime_power_pred ) { // i.e. coefficient is not zero
        if ( f == prime_power_pred ) { // i.e. f = 0
          f = poly_coeff_exponents[dx] + xpw;
          f = exponent_reduction_table[f];
        } else {
          unsigned int tmp = exponent_reduction_table[poly_coeff_exponents[dx] + xpw];

          unsigned int tmp2;
          if (tmp <= f) {
            // todo: this can be removed by doubling the size of incrementation_table
            // and checking at prime_power_pred + 1 + tmp - f
            tmp2 = f;
            f = tmp;
            tmp = tmp2;
          }
          tmp2 = incrementation_table[tmp-f];
          if ( tmp2 != prime_power_pred ) {
            f = f + tmp2;
            f = exponent_reduction_table[f];
          } else
            f = prime_power_pred;
        }
      }
    }

    if ( f == prime_power_pred )
      get<1>(this->nmb_points[prime_exponent]) += 1;
    else if ( !(f & 1) )
      get<0>(this->nmb_points[prime_exponent]) += 2;
  }
}

void
Curve::
count_naive_nmod(
    unsigned int prime_exponent
  )
{
  if ( prime_exponent <= 0 ) {
    cerr << "Curve.count_naive: prime exponent must be positive: "
         << prime_exponent << endl;
    throw;
  }
  if ( prime_exponent % this->table->prime_exponent != 0 ) {
    cerr << "Curve.count_naive: prime exponent must divide curve prime exponent: "
         << prime_exponent << " " << this->table->prime_exponent << endl;
    throw;
  }

  if ( this->nmb_points.find( prime_exponent ) != this->nmb_points.end() )
    return;


  int prime_power_pred = pow(table->prime, prime_exponent) - 1;


  // create fq_nmod_ctx and gen
  fq_nmod_ctx_t fq_ctx;
  fmpz_t prime_fmpz;
  fmpz_init_set_ui(prime_fmpz, this->table->prime);
  // use Conway polynomials so that fq_nmod_gen is a multiplicative generator
  fq_nmod_ctx_init_conway(fq_ctx, prime_fmpz, prime_exponent, ((string)"T").c_str());
  fmpz_clear(prime_fmpz);

  fq_nmod_t gen;
  fq_nmod_init(gen, fq_ctx);
  fq_nmod_gen(gen, fq_ctx);

  fq_nmod_t sub_gen;
  fq_nmod_init(sub_gen, fq_ctx);
  fq_nmod_pow_ui(sub_gen, gen, prime_power_pred / this->table->prime_power_pred, fq_ctx);

  // convert coefficient exponents to fq_nmod elements
  // fixme: in this conversion as in the other ones, we
  // silently assume that gen_q = gen_{q^l}^{q^l - q}
  vector<const fq_nmod_struct*> poly_coefficients;
  for ( unsigned int e : this->poly_coeff_exponents ) {
    auto a = new fq_nmod_struct;
    fq_nmod_init(a, fq_ctx);
    if ( e == table->prime_power_pred )
      fq_nmod_zero(a, fq_ctx);
    else
      fq_nmod_pow_ui(a, sub_gen, e, fq_ctx);
    poly_coefficients.push_back(a);
  }


  // detection of squares in the finite field
  int square_detection_exp = prime_power_pred >> 1;
  fq_nmod_t square_detection_tmp;
  fq_nmod_init(square_detection_tmp, fq_ctx);
  auto is_square_nonzero =
    [&fq_ctx, square_detection_exp, &square_detection_tmp]
    (const fq_nmod_t a)
    {
      fq_nmod_pow_ui(square_detection_tmp, a, square_detection_exp, fq_ctx);
      return fq_nmod_is_one(square_detection_tmp, fq_ctx);
    };


  // points x != infty
  fq_nmod_t x, xpw, rhs, m;
  fq_nmod_init(x, fq_ctx);
  fq_nmod_init(xpw, fq_ctx);
  fq_nmod_init(rhs, fq_ctx);
  fq_nmod_init(m, fq_ctx);

  auto & nmb_points = this->nmb_points[prime_exponent];
  auto update_count =
    [&fq_ctx, &x, &xpw, &rhs, &m,
     &poly_coefficients, &nmb_points,
     &is_square_nonzero]()
    {
      fq_nmod_one(xpw, fq_ctx);
      fq_nmod_zero(rhs, fq_ctx);
      for ( auto c : poly_coefficients ) {
        fq_nmod_mul(m, c, xpw, fq_ctx);
        fq_nmod_add(rhs, rhs, m, fq_ctx);
        fq_nmod_mul(xpw, xpw, x, fq_ctx);
      }

      if ( fq_nmod_is_zero(rhs, fq_ctx) )
        get<1>(nmb_points) += 1;
      else if ( is_square_nonzero(rhs) )
        get<0>(nmb_points) += 2;
    };

  fq_nmod_zero(x, fq_ctx);
  update_count();
  fq_nmod_one(x, fq_ctx);
  for ( int ix = 0; ix < prime_power_pred; ++ix ) {
    update_count();
    fq_nmod_mul(x, x, gen, fq_ctx);
  }


  // point x = infty
  // if poly_coeffs ends with zero entry
  if ( this->degree() < 2*this->genus() + 2 )
    get<1>(this->nmb_points[prime_exponent]) += 1;
  // if leading coefficient is odd power of generator
  else if ( is_square_nonzero(poly_coefficients.back()) )
    get<0>(this->nmb_points[prime_exponent]) += 2;


  // clear flint variables
  fq_nmod_clear(gen, fq_ctx);
  fq_nmod_clear(sub_gen, fq_ctx);
  fq_nmod_clear(x, fq_ctx);
  fq_nmod_clear(xpw, fq_ctx);
  fq_nmod_clear(rhs, fq_ctx);
  fq_nmod_clear(m, fq_ctx);
  fq_nmod_clear(square_detection_tmp, fq_ctx);
  fq_nmod_ctx_clear(fq_ctx);
}

void
Curve::
count_naive_zech(
    unsigned int prime_exponent
  )
{
  if ( prime_exponent <= 0 ) {
    cerr << "Curve.count_naive: prime exponent must be positive: "
         << prime_exponent << endl;
    throw;
  }
  if ( prime_exponent % this->table->prime_exponent != 0 ) {
    cerr << "Curve.count_naive: prime exponent must divide curve prime exponent: "
         << prime_exponent << " " << this->table->prime_exponent << endl;
    throw;
  }

  if ( this->nmb_points.find( prime_exponent ) != this->nmb_points.end() )
    return;


  int prime_power_pred = pow(table->prime, prime_exponent) - 1;


  // create fq_zech_ctx and gen
  fq_zech_ctx_t fq_ctx;
  fmpz_t prime_fmpz;
  fmpz_init_set_ui(prime_fmpz, this->table->prime);
  fq_zech_ctx_init(fq_ctx, prime_fmpz, prime_exponent, ((string)"T").c_str());
  fmpz_clear(prime_fmpz);

  fq_zech_t gen;
  fq_zech_init(gen, fq_ctx);
  fq_zech_gen(gen, fq_ctx);

  fq_zech_t sub_gen;
  fq_zech_init(sub_gen, fq_ctx);
  fq_zech_pow_ui(sub_gen, gen, prime_power_pred / this->table->prime_power_pred, fq_ctx);

  // convert coefficient exponents to fq_zech elements
  // fixme: in this conversion as in the other ones, we
  // silently assume that gen_q = gen_{q^l}^{q^l - q}
  vector<const fq_zech_struct*> poly_coefficients;
  for ( unsigned int e : this->poly_coeff_exponents ) {
    auto a = new fq_zech_struct;
    fq_zech_init(a, fq_ctx);
    if ( e == table->prime_power_pred )
      fq_zech_zero(a, fq_ctx);
    else
      fq_zech_pow_ui(a, sub_gen, e, fq_ctx);
    poly_coefficients.push_back(a);
  }


  // detection of squares in the finite field
  int square_detection_exp = prime_power_pred >> 1;
  fq_zech_t square_detection_tmp;
  fq_zech_init(square_detection_tmp, fq_ctx);
  auto is_square_nonzero =
    [&fq_ctx, square_detection_exp, &square_detection_tmp]
    (const fq_zech_t a)
    {
      fq_zech_pow_ui(square_detection_tmp, a, square_detection_exp, fq_ctx);
      return fq_zech_is_one(square_detection_tmp, fq_ctx);
    };


  // points x != infty
  fq_zech_t x, xpw, rhs, m;
  fq_zech_init(x, fq_ctx);
  fq_zech_init(xpw, fq_ctx);
  fq_zech_init(rhs, fq_ctx);
  fq_zech_init(m, fq_ctx);

  auto & nmb_points = this->nmb_points[prime_exponent];
  auto update_count =
    [&fq_ctx, &x, &xpw, &rhs, &m,
     &poly_coefficients, &nmb_points,
     &is_square_nonzero]()
    {
      fq_zech_one(xpw, fq_ctx);
      fq_zech_zero(rhs, fq_ctx);
      for ( auto c : poly_coefficients ) {
        fq_zech_mul(m, c, xpw, fq_ctx);
        fq_zech_add(rhs, rhs, m, fq_ctx);
        fq_zech_mul(xpw, xpw, x, fq_ctx);
      }

      if ( fq_zech_is_zero(rhs, fq_ctx) )
        get<1>(nmb_points) += 1;
      else if ( is_square_nonzero(rhs) )
        get<0>(nmb_points) += 2;
    };

  fq_zech_zero(x, fq_ctx);
  update_count();
  fq_zech_one(x, fq_ctx);
  for ( int ix = 0; ix < prime_power_pred; ++ix ) {
    update_count();
    fq_zech_mul(x, x, gen, fq_ctx);
  }


  // point x = infty
  // if poly_coeffs ends with zero entry
  if ( this->degree() < 2*this->genus() + 2 )
    get<1>(this->nmb_points[prime_exponent]) += 1;
  // if leading coefficient is odd power of generator
  else if ( is_square_nonzero(poly_coefficients.back()) )
    get<0>(this->nmb_points[prime_exponent]) += 2;


  // clear flint variables
  fq_zech_clear(gen, fq_ctx);
  fq_zech_clear(sub_gen, fq_ctx);
  fq_zech_clear(x, fq_ctx);
  fq_zech_clear(xpw, fq_ctx);
  fq_zech_clear(rhs, fq_ctx);
  fq_zech_clear(m, fq_ctx);
  fq_zech_clear(square_detection_tmp, fq_ctx);
  fq_zech_ctx_clear(fq_ctx);
}

vector<tuple<unsigned int,unsigned int>>
Curve::
number_of_points(
    unsigned int max_prime_exponent
    )
  const
{
  unsigned int prime_exponent = this->table->prime_exponent;

  vector<tuple<unsigned int,unsigned int>> nmb_points;
  nmb_points.reserve(max_prime_exponent/prime_exponent);
  for ( size_t fx=prime_exponent;
        fx<=max_prime_exponent;
        fx+=prime_exponent )
    nmb_points.push_back(this->nmb_points.at(fx));

  return nmb_points;
}


unsigned int
Curve::
max_prime_exponent()
  const
{
  // DEBUG
  /*
  cerr << "DEBUG: " << endl;
  for ( auto p : this->nmb_points )
    cerr << get<0>(p) << " " << get<0>(get<1>(p)) << " " << get<1>(get<1>(p)) << endl;
  */

  unsigned int max_prime_exponent = this->prime_exponent();
  while ( this->nmb_points.count(max_prime_exponent) != 0 )
    max_prime_exponent += this->prime_exponent();

  return max_prime_exponent - this->prime_exponent();
}

map<unsigned int, int>
Curve::
hasse_weil_offsets()
  const
{
  map<unsigned int, int> offsets;
  for ( auto & pts_it : this->nmb_points )
    offsets[pts_it.first] =   pow(this->table->prime, pts_it.first) + 1
                            - (get<0>(pts_it.second) + get<1>(pts_it.second));

  return offsets;
}

vector<int>
Curve::
hasse_weil_offsets(
    unsigned int max_prime_exponent
    )
  const
{
  unsigned int prime_exponent = this->table->prime_exponent;
  auto offset_map = this->hasse_weil_offsets();

  vector<int> offsets;
  offsets.reserve(max_prime_exponent / prime_exponent);
  for ( size_t fx=prime_exponent;
        fx<=max_prime_exponent;
        fx+=prime_exponent )
    offsets.push_back(offset_map[fx]);

  return offsets;
}

vector<unsigned int>
Curve::
ramification_type()
  const
{
  vector<unsigned int> ramifications;

  // try to compute ramification from point counts

  unsigned int ramification_sum = 0;
  map<unsigned int, unsigned int> nmb_ramified_points_from_lower;
  for ( unsigned int fx = 1; fx < this->degree(); ++fx )
    nmb_ramified_points_from_lower[fx] = 0;

  unsigned int fx;
  for ( fx = 1; fx < this->degree(); ++fx ) {
    auto nmb_points_it = this->nmb_points.find(fx*this->prime_exponent());
    if ( nmb_points_it == this->nmb_points.end() )
      break;
   
    auto nmb_ramified_points_new =
        get<1>(nmb_points_it->second) - nmb_ramified_points_from_lower[fx];

    ramification_sum += nmb_ramified_points_new;
    for ( size_t jx = 0; jx < nmb_ramified_points_new; jx += fx )
      ramifications.push_back(fx);

    for ( size_t gx = fx; gx < this->degree(); gx += fx )
      nmb_ramified_points_from_lower[gx] += nmb_ramified_points_new;
  }


  unsigned int ramification_difference = 2*this->genus() + 2 - ramification_sum;
  if ( ramification_difference < 2*fx ) {
    if ( ramification_difference != 0 )
      ramifications.push_back(ramification_difference);
    return ramifications;
  }


  // if ramification can not be computed from available point count,
  // factor the right hand side polynomial

  ramifications.clear();
  if ( this->degree() % 2 == 1 )
    ramifications.push_back(1);
  if ( this->table->is_prime_field() ) {
    auto poly = this->rhs_nmod_polynomial();
    nmod_poly_factor_t poly_factor;
    nmod_poly_factor_init(poly_factor);
    nmod_poly_factor(poly_factor, &poly);

    for (unsigned int ix=0; ix<(unsigned int)poly_factor->num; ++ix)
      for (unsigned int jx=0; jx<(unsigned int)poly_factor->exp[ix]; ++jx)
        ramifications.push_back(nmod_poly_degree(poly_factor->p + ix));

    nmod_poly_factor_clear(poly_factor);
    nmod_poly_clear(&poly);
  }
  else {
    auto poly = this->rhs_polynomial();
    fq_nmod_t lead;
    fq_nmod_init(lead, this->table->fq_ctx);
    fq_nmod_poly_factor_t poly_factor;
    fq_nmod_poly_factor_init(poly_factor, this->table->fq_ctx);
    fq_nmod_poly_factor(poly_factor, lead, &poly, this->table->fq_ctx);

    for (unsigned int ix=0; ix<(unsigned int)poly_factor->num; ++ix)
      for (unsigned int jx=0; jx<(unsigned int)poly_factor->exp[ix]; ++jx)
        ramifications.push_back(fq_nmod_poly_degree(poly_factor->poly + ix, this->table->fq_ctx));

    fq_nmod_poly_factor_clear(poly_factor, this->table->fq_ctx);
    fq_nmod_poly_clear(&poly, this->table->fq_ctx);
    fq_nmod_clear(lead, this->table->fq_ctx);
  }

  sort(ramifications.begin(), ramifications.end());
  return ramifications;
}

nmod_poly_struct
Curve::
rhs_nmod_polynomial()
  const
{
  if ( this->table->prime_exponent != 1 ) {
    cerr << "Conversion to nmod_poly_t only possible for prime fields" << endl;
    throw;
  }

  nmod_poly_struct poly;
  nmod_poly_init2( &poly, this->table->prime, this->poly_coeff_exponents.size() );

  for (long int ix=0; ix<(long int)this->poly_coeff_exponents.size(); ++ix)
    nmod_poly_set_coeff_ui(&poly, ix, this->table->at_nmod(this->poly_coeff_exponents[ix]));

  return poly;
}

fq_nmod_poly_struct
Curve::
rhs_polynomial()
  const
{
  fq_nmod_poly_struct poly;
  fq_nmod_poly_init2( &poly, this->poly_coeff_exponents.size(), this->table->fq_ctx );

  for (long int ix=0; ix<(long int)this->poly_coeff_exponents.size(); ++ix)
    fq_nmod_poly_set_coeff( &poly, ix, this->table->at(this->poly_coeff_exponents[ix]), this->table->fq_ctx );

  return poly;
}
