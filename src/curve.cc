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
    const vector<int> poly_coeff_exponents
    ) :
    table( table )
{
  this->poly_coeff_exponents = move(poly_coeff_exponents);

  while ( this->poly_coeff_exponents.back() == this->table->zero_index() )
    this->poly_coeff_exponents.pop_back();
}

int
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
_support(
    shared_ptr<FqElementTable> table,
    vector<int> poly_coeff_exponents
    )
{
  vector<unsigned int> support;

  for ( size_t ix=0; ix<poly_coeff_exponents.size(); ++ix )
    if ( !table->is_zero(poly_coeff_exponents[ix]) )
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

vector<int>
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

  vector<int> converted;
  converted.reserve(this->poly_coeff_exponents.size());
  for ( int c : this->poly_coeff_exponents )
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
  int prime_exponent = reduction_table.prime_exponent;

  if ( this->nmb_points.find( prime_exponent ) != this->nmb_points.end() )
    return;
  for (size_t fx=1; fx<=prime_exponent; ++fx)
    if ( prime_exponent % fx == 0 )
      this->nmb_points[fx] = make_tuple(0,0);


  // this also checks that the prime exponent is divisible by the one of the curve
  const vector<int> poly_coeff_exponents = this->convert_poly_coeff_exponents(reduction_table);

  // ponts x != 0, infty
  if ( reduction_table.is_opencl_enabled() )
    this->count_opencl(reduction_table, poly_coeff_exponents);
  else
    this->count_cpu(reduction_table, poly_coeff_exponents);


  // point x = 0
  // if constant coefficient is zero
  if (poly_coeff_exponents.front() == reduction_table.prime_power_pred)
    for ( size_t fx=1; fx<=prime_exponent; ++fx)
      ++get<1>(this->nmb_points[fx]);
  // if constant coefficient is even power of generator
  else if (!(poly_coeff_exponents.front() & 1)) {
    size_t fx_min = (*reduction_table.minimal_field_table)[poly_coeff_exponents.front() / 2] + 1;
    for ( size_t fx=fx_min; fx<=prime_exponent; fx+=fx_min )
      if ( prime_exponent % fx == 0 )
        get<0>(this->nmb_points[fx]) += 2;
  }


  // point x = infty
  // if poly_coeffs ends with zero entry
  if ( this->degree() < 2*this->genus() + 2 )
    for ( size_t fx=1; fx<=prime_exponent; ++fx)
      get<1>(this->nmb_points[fx]) += 1;
  // f leading coefficient is odd power of generator
  else if (!(poly_coeff_exponents.back() & 1)) {
    // todo: make sure that curve enumeration does not put 0 as the leading coefficient
    size_t fx_min = (*reduction_table.minimal_field_table)[poly_coeff_exponents.back() / 2] + 1;
    for ( size_t fx=fx_min; fx<=prime_exponent; fx+=fx_min )
      if ( prime_exponent % fx == 0 )
        get<0>(this->nmb_points[fx]) += 2;
  }
}

void
Curve::
count_opencl(
    ReductionTable & reduction_table,
    const vector<int> & poly_coeff_exponents
    )
{
#ifdef WITH_OPENCL
  reduction_table.kernel_evaluation(this->degree())->enqueue(poly_coeff_exponents);
  reduction_table.kernel_reduction()->reduce(this->nmb_points);
#else
  cerr << "Curve::count_opencl: compiled without OpenCL support" << endl;
  throw;
#endif
}

void
Curve::
count_cpu(
    const ReductionTable & reduction_table,
    const vector<int> & poly_coeff_exponents
    )
{
  int prime_exponent = reduction_table.prime_exponent;
  int prime_power_pred = reduction_table.prime_power_pred;

  const auto & exponent_reduction_table = *reduction_table.exponent_reduction_table;
  const auto & incrementation_table = *reduction_table.incrementation_table;
  const auto & minimal_field_table = *reduction_table.minimal_field_table;

  int poly_size = (int)poly_coeff_exponents.size();


  for ( int x = 1; x <= prime_power_pred; ++x ) {
    int f = poly_coeff_exponents[0];
    for ( int dx=1, xpw=x; dx < poly_size; ++dx, xpw+=x ) {
      xpw = exponent_reduction_table[xpw];
      if ( poly_coeff_exponents[dx] != prime_power_pred ) { // i.e. coefficient is not zero
        if ( f == prime_power_pred ) { // i.e. f = 0
          f = poly_coeff_exponents[dx] + xpw;
          f = exponent_reduction_table[f];
        } else {
          int tmp = exponent_reduction_table[poly_coeff_exponents[dx] + xpw];

          int tmp2;
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


    int minimal_field_x = 1 + minimal_field_table[x];
    if ( f == prime_power_pred ) {
      // minimal_field_f = 0;

      for ( size_t fx=minimal_field_x; fx<=prime_exponent; fx+=minimal_field_x )
        if ( prime_exponent % fx == 0 )
          ++get<1>(this->nmb_points[fx]);
    }
    else if ( !(f & 1) ) {
      int minimal_field_f = 1 + minimal_field_table[f/2];
      minimal_field_x = minimal_field_x > minimal_field_f ?  minimal_field_x : minimal_field_f;

      for ( size_t fx=minimal_field_x; fx<=prime_exponent; fx+=minimal_field_x )
        if ( prime_exponent % fx == 0 )
          get<0>(this->nmb_points[fx]) += 2;
    }
  }
}

vector<tuple<int,int>>
Curve::
number_of_points(
    unsigned int max_prime_exponent
    )
  const
{
  unsigned int prime_exponent = this->table->prime_exponent;

  vector<tuple<int,int>> nmb_points;
  nmb_points.reserve(max_prime_exponent/prime_exponent);
  for ( size_t fx=prime_exponent;
        fx<=max_prime_exponent;
        fx+=prime_exponent )
    nmb_points.push_back(this->nmb_points.at(fx));

  return nmb_points;
}

map<unsigned int, int>
Curve::
hasse_weil_offsets()
  const
{
  if ( !this->table->is_prime_field() ) {
    cerr << "hasse_weil_offsets implemented only for prime fields" << endl;
    throw;
    // todo: implement
  }

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

vector<int>
Curve::
ramification_type()
  const
{
  vector<int> ramifications;

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


  int ramification_difference = 2*this->genus() + 2 - ramification_sum;
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

    for (size_t ix=0; ix<poly_factor->num; ++ix)
      for (size_t jx=0; jx<poly_factor->exp[ix]; ++jx)
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

    for (size_t ix=0; ix<poly_factor->num; ++ix)
      for (size_t jx=0; jx<poly_factor->exp[ix]; ++jx)
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

  for (long ix=0; ix<this->poly_coeff_exponents.size(); ++ix)
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

  for (long ix=0; ix<this->poly_coeff_exponents.size(); ++ix)
    fq_nmod_poly_set_coeff( &poly, ix, this->table->at(this->poly_coeff_exponents[ix]), this->table->fq_ctx );

  return poly;
}
