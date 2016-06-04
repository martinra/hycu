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


#include <set>

#include "curve_iterator.hh"

#include "flint/fmpz.h"


using namespace std;

// debug:
bool print_debug = false;


CurveIterator::
CurveIterator(
    const FqElementTable & table,
    int genus,
    unsigned int package_size
    ) :
  prime ( table.prime )
{
  for ( int degree = 2*genus + 1; degree < 2*genus + 3; ++degree ) {
    // next to hightest coefficient is zero
    // two consecutive coefficients depend on each other
    // and vary between a square and non-square
    
    // ix is exponent of the second non-zero coefficient
    for ( int ix=degree-2; ix>=0; --ix ) {
      // jx is the exponent of the non-zero coefficient thereafter
      for ( int jx=ix-1; jx>=-1; --jx ) {
        map<size_t, tuple<int,int>> blocks;
        map<size_t, vector<int>> sets;
        map<size_t, tuple<size_t, map<int, vector<int>>>> dependent_sets;

        // highest coefficient is nonzero
        blocks[degree] = table.block_non_zero();
        for ( size_t kx=degree-1; kx>ix; --kx )
          sets[kx] = {table.zero_index()};


        // second nonzero coefficient is determined up to squaring
        // it suffices, however, to choose a square here, since the other
        // curves are twists, which we do not need enumerate separately
        sets[ix] = table.power_coset_representatives(1);
        for ( int kx=ix-1; kx>jx; --kx )
          sets[kx] = {table.zero_index()};

        // the third nonzero coefficient is coupled with the second one
        if ( jx != -1 ) {
          // its possible values depend on the difference of exponents to the
          // previous one
          auto cosets = table.power_coset_representatives(ix-jx);

          map<int, vector<int>> coset_products;
          set<int> coset_product;

          for ( size_t sx=0; sx<sets[ix].size(); ++sx ) {
            coset_product.clear();
            for ( int b : cosets )
              coset_product.insert(table.reduce_generator_exponent(sets[ix][sx]+b));
            coset_products[sx] = vector<int>(coset_product.cbegin(), coset_product.cend());
          }
          dependent_sets[jx] = make_tuple(ix, coset_products);
        }

        // remaining coefficients are free to vary
        for ( int kx = jx-1; kx>=0; --kx )
          blocks[kx] = table.block_complete();

        this->enumerators.emplace_back(
            degree+1, blocks, package_size, sets, dependent_sets );
      }
    }

    { // the case of x^degree
      map<size_t, vector<int>> sets;
      // first coefficient is determined up to squaring
      // it suffices, however, to choose a square here, since the other
      // curves are twists, which we do not need enumerate separately
      sets[degree] = table.power_coset_representatives(1);
      for (int kx=degree-1; kx>=0; --kx)
        sets[kx] = {table.zero_index()};

      this->enumerators.emplace_back(
              BlockIterator(degree+1, {}, package_size, sets, {}) );
    }
  }

  this->enumerator_it = this->enumerators.begin();
}

CurveIterator const&
CurveIterator::
step()
{
  if (this->is_end()) return *this;

  this->enumerator_it->step();
  if ( this->enumerator_it->is_end() )
    ++this->enumerator_it;

  return *this;
}

bool
CurveIterator::
is_end()
  const
{
  return ( this->enumerator_it == this->enumerators.end() );
}

unsigned int
CurveIterator::
multiplicity(
    const Curve & curve
    )
{
  unsigned int prime = curve.prime();
  unsigned int prime_power = curve.prime_power();
  unsigned int prime_power_pred = prime_power - 1;
  unsigned int degree = curve.degree();

  if ( prime <= degree || prime <= 3 ) {
    cerr << "CurveIterator::multiplicity: "
         << "curve iterator implemented only if prime is larger than rhs degree and larger than 3" << endl;
    throw;
  }


  const auto & support = curve.rhs_support();
  if ( support.size() <= 2 )
    // the orbit of a_n x^n consists of a_n b_2^2 (x+b_1)^n
    return prime_power * prime_power_pred / 2;
  else {
    // the orbit of a_n x^n + a_m x^m + a_l x^l + ....
    // is b_2^2 ( a_n (b_3 x+b_1)^n + a_m (b_3 x+b_1)^m + a_l (b_3 x+b_1)^l + .... )
    // the orbit size comes from the (n-1)-th coefficent, the m-th coeffficent and the qutient of the
    // m-th by the l-th one
    unsigned int snd_dx = *(support.rbegin()+1);
    unsigned int trd_dx = *(support.rbegin()+2);

    return   prime_power * prime_power_pred / 2
           * prime_power_pred / n_gcd(prime_power_pred, snd_dx-trd_dx);
  }
}

bool
CurveIterator::
is_reduced(
    const Curve & curve
    )
{
  unsigned int prime = curve.prime();
  unsigned int prime_power_pred = curve.prime_power() - 1;
  unsigned int degree = curve.degree();

  if ( prime <= degree || prime <= 3 ) {
    cerr << "CurveIterator::is_partially_reduced: "
         << "curve iterator implemented only if prime is larger than rhs degree and larger than 3" << endl;
    throw;
  }


  const auto & base_field_table = curve.base_field_table();

  const auto & poly_coeff_exponents = curve.rhs_coeff_exponents();
  if ( !base_field_table->is_zero(poly_coeff_exponents[degree-1]) )
    return false;

  const auto & support = curve.rhs_support();
  if ( support.size() <= 2 )
    return base_field_table->is_power_coset_representative( 2,
                                          poly_coeff_exponents[support.front()] );
  else {
    unsigned int snd_dx = *(support.crbegin()+1);
    unsigned int trd_dx = *(support.crbegin()+2);
    int snd_coeff = poly_coeff_exponents[snd_dx];
    int trd_coeff = poly_coeff_exponents[trd_dx];

    return    base_field_table->is_power_coset_representative(2, snd_coeff)
           && base_field_table->is_power_coset_representative(snd_dx-trd_dx, trd_coeff-snd_coeff);
  }
}

Curve
CurveIterator::
reduce(
    const Curve & curve
    )
{
  auto base_field_table = curve.base_field_table();


  vector<int> rhs_shifted;
  if ( base_field_table->is_zero(curve.rhs_coeff_exponents()[curve.degree()-1]) ) {
    rhs_shifted = curve.rhs_coeff_exponents();
  }
  else {
    // additive reduction
    // we could use FLINT for this, but composition of polynomials hangs
    auto fq_nmod_ctx = base_field_table->fq_nmod_ctx();
  
    vector<fq_nmod_struct*> fq_rhs = curve.rhs_coefficients();
  
    fq_nmod_t shift;
    fq_nmod_init(shift, fq_nmod_ctx);
    fq_nmod_set(shift, fq_rhs[curve.degree()-1], fq_nmod_ctx);
  
    fq_nmod_t tmp;
    fq_nmod_init(tmp, fq_nmod_ctx);
  
    // divide by degree
    fq_nmod_set_ui(tmp, curve.degree(), fq_nmod_ctx);
    // cerr << "reduce " << curve << endl;
    fq_nmod_inv(tmp, tmp, fq_nmod_ctx);
    // cerr << "first inv done" << endl;
    fq_nmod_mul(shift, shift, tmp, fq_nmod_ctx);
  
    // divide by highest coefficient
    fq_nmod_set(tmp, fq_rhs.back(), fq_nmod_ctx);
    // cerr << "second inv" << endl;
    fq_nmod_inv(tmp, tmp, fq_nmod_ctx);
    // cerr << "second inv done" << endl;
    fq_nmod_mul(shift, shift, tmp, fq_nmod_ctx);
  
    fq_nmod_neg(shift, shift, fq_nmod_ctx);
  

    rhs_shifted = CurveFq( base_field_table,
                           CurveIterator::x_shift(base_field_table, fq_rhs, shift) )
                    .rhs_coeff_exponents();
  
    for ( auto c : fq_rhs ) {
      fq_nmod_clear(c, fq_nmod_ctx);
      delete c;
    }
    fq_nmod_clear(shift, fq_nmod_ctx);
  }

  return CurveIterator::reduce_multiplicative(Curve(base_field_table, move(rhs_shifted)));
}

Curve
CurveIterator::
reduce_multiplicative(
    const Curve & curve
    )
{
  const auto base_field_table = curve.base_field_table();
  auto poly_coeff_exponents = curve.rhs_coeff_exponents();
  const auto & support = curve.rhs_support();

  // first normalize the difference between the second and third nonvanishing coefficient
  // by means of x -> (b*x)
  if ( support.size() > 2 ) {
    unsigned int snd_dx = *(support.crbegin()+1);
    unsigned int trd_dx = *(support.crbegin()+2);
    int snd_coeff = poly_coeff_exponents[snd_dx];
    int trd_coeff = poly_coeff_exponents[trd_dx];

    if ( print_debug ) {
      cerr << snd_coeff << " " << trd_coeff << endl;
    }

    int coeff_diff = trd_coeff-snd_coeff;
    unsigned int exponent_diff = snd_dx-trd_dx;
    int coeff_diff_representative =
      base_field_table->power_coset_representative(exponent_diff, coeff_diff);
    int reduce_diff = base_field_table->reduce_generator_exponent(
          ( coeff_diff - coeff_diff_representative ) / exponent_diff );
    if ( print_debug ) {
      cerr << reduce_diff << endl;
    }
    for ( auto ix : support )
      poly_coeff_exponents[ix] = base_field_table->reduce_generator_exponent(poly_coeff_exponents[ix] + ix*reduce_diff);
  }


  // second normalize by means of y -> (b*y)^2
  unsigned int reduction_ix;
  if ( support.size() <= 2 )
    reduction_ix = support.front(); 
  else
    reduction_ix = *(support.crbegin()+1);

  int reduce_diff =
        base_field_table->power_coset_representative(2, poly_coeff_exponents[reduction_ix])
      - poly_coeff_exponents[reduction_ix];

  for ( auto ix : support )
    poly_coeff_exponents[ix] = base_field_table->reduce_generator_exponent(poly_coeff_exponents[ix] + reduce_diff);


  return Curve(base_field_table, move(poly_coeff_exponents));
}

set<vector<int>>
CurveIterator::
orbit(
    const Curve & curve,
    map<vector<int>, unsigned int> orbits
    )
{
  if ( orbits.count(curve.rhs_coeff_exponents()) == 1 )
    return set<vector<int>>();

  const auto base_field_table = curve.base_field_table();

  set<vector<int>> new_orbit;
  vector<tuple<vector<int>,vector<fq_nmod_struct*>>> unexpanded_polys;
  set<vector<int>> unexpanded_polys_set;
  unexpanded_polys.push_back(
      make_tuple( curve.rhs_coeff_exponents(), curve.rhs_coefficients() ) );
  unexpanded_polys_set.insert(curve.rhs_coeff_exponents());

  vector<int> poly;
  vector<fq_nmod_struct*> fq_poly;
  while ( !unexpanded_polys.empty() ) {
    tie(poly, fq_poly) = unexpanded_polys.back();
    unexpanded_polys.pop_back();
    unexpanded_polys_set.erase(poly);
    new_orbit.insert(move(poly));


    for ( unsigned int shift=0; shift<curve.prime_power()-1; ++shift ) {
      auto curve_shifted =
        CurveIterator::reduce_multiplicative( CurveFq(
            base_field_table,
            CurveIterator::x_shift(base_field_table, fq_poly, shift) ) );
      auto poly_shifted = curve_shifted.rhs_coeff_exponents();
      auto fq_poly_shifted = curve_shifted.rhs_coefficients();

      if ( orbits.count(poly_shifted) == 1 )
        return set<vector<int>>();
      if (  new_orbit.count(poly_shifted) == 0
            && unexpanded_polys_set.count(poly_shifted) == 0 ) {
        unexpanded_polys_set.insert(poly_shifted);
        unexpanded_polys.push_back(
            make_tuple(move(poly_shifted), move(fq_poly_shifted)) );
      }
    }

    for ( unsigned int shift=0; shift<curve.prime_power()-1; ++shift ) {
      auto curve_shifted =
        CurveIterator::reduce_multiplicative( CurveFq(
            base_field_table,
            CurveIterator::z_shift(base_field_table, fq_poly, shift) ) );
      auto poly_shifted = curve_shifted.rhs_coeff_exponents();
      auto fq_poly_shifted = curve_shifted.rhs_coefficients();

      if ( orbits.count(poly_shifted) == 1 )
        return set<vector<int>>();
      if (  new_orbit.count(poly_shifted) == 0
            && unexpanded_polys_set.count(poly_shifted) == 0 )
        unexpanded_polys_set.insert(poly_shifted);
        unexpanded_polys.push_back(
            make_tuple(move(poly_shifted), move(fq_poly_shifted)) );
    }
  }

  return new_orbit;
}

vector<fq_nmod_struct*>
CurveIterator::
z_shift(
    const shared_ptr<FqElementTable> base_field_table,
    const vector<fq_nmod_struct*> & poly,
    const fq_nmod_t shift
    )
{
  unsigned int size_add = poly.size() & 1 ? 0 : 1;
  vector<fq_nmod_struct*> poly_reverted(poly.size() + size_add);

  if ( !(poly.size() & 1) ) {
    auto a = new fq_nmod_struct;
    fq_nmod_init(a, base_field_table->fq_nmod_ctx());
    poly_reverted[0] = a;
  }

  copy( poly.crbegin(), poly.crend(),
        poly_reverted.begin() + size_add );

  const auto & poly_shifted_reversed =
    CurveIterator::_shift_polynomial(
        poly_reverted, shift, base_field_table->fq_nmod_ctx() );

  if ( fq_nmod_is_zero(poly_shifted_reversed[0], base_field_table->fq_nmod_ctx()) )
    return vector<fq_nmod_struct*>( poly_shifted_reversed.crbegin(),
                                    poly_shifted_reversed.crbegin()
                                      + (poly_shifted_reversed.size()-1) );
  else
    return vector<fq_nmod_struct*>( poly_shifted_reversed.crbegin(),
                                    poly_shifted_reversed.crend() );
}

vector<fq_nmod_struct*>
CurveIterator::
_shift_polynomial(
  const vector<fq_nmod_struct*> & fq_poly,
  const fq_nmod_t shift,
  const fq_nmod_ctx_t fq_nmod_ctx
  )
{
  char * shift_str;
  fq_nmod_t shift_c;
  fq_nmod_init(shift_c, fq_nmod_ctx);
  fq_nmod_set(shift_c, shift, fq_nmod_ctx);
  fq_nmod_reduce(shift_c, fq_nmod_ctx);
  shift_str = fq_nmod_get_str_pretty(shift_c, fq_nmod_ctx);
  // cerr << "shift: " << string(shift_str) << endl;
  flint_free(shift_str);

  fmpz_t binomial;
  fmpz_init(binomial);

  fq_nmod_t shift_pw;
  fq_nmod_init(shift_pw, fq_nmod_ctx);
  fq_nmod_set(shift_pw, shift, fq_nmod_ctx);

  fq_nmod_t tmp;
  fq_nmod_init(tmp, fq_nmod_ctx);


  // contribution of shift^0
  vector<fq_nmod_struct*> fq_poly_shifted;
  fq_poly_shifted.reserve(fq_poly.size());
  for ( auto c : fq_poly ) {
    auto fq_coeff = new fq_nmod_struct;
    fq_nmod_init(fq_coeff, fq_nmod_ctx);
    fq_nmod_set(fq_coeff, c, fq_nmod_ctx);
    fq_poly_shifted.push_back(fq_coeff);
  }

  // contribution of shift^dx
  for ( unsigned int dx=1; dx<fq_poly.size(); ++dx ) {
    // from the term (x + shift)^ox
    for ( unsigned int ox=fq_poly.size()-1; ox>=dx; --ox ) {
      fmpz_bin_uiui(binomial, ox, dx);
      fq_nmod_set_ui(tmp, fmpz_get_ui(binomial) , fq_nmod_ctx);
      fq_nmod_mul(tmp, tmp, shift_pw, fq_nmod_ctx);
      fq_nmod_mul(tmp, tmp, fq_poly[ox], fq_nmod_ctx);
      fq_nmod_add(fq_poly_shifted[ox-dx], fq_poly_shifted[ox-dx], tmp, fq_nmod_ctx);
    }

    fq_nmod_mul(shift_pw, shift_pw, shift, fq_nmod_ctx);
  }


  fmpz_clear(binomial);
  fq_nmod_clear(shift_pw, fq_nmod_ctx);
  fq_nmod_clear(tmp, fq_nmod_ctx);


  return fq_poly_shifted;
}
