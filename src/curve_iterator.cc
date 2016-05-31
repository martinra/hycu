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

#include "curve.hh"
#include "curve_iterator.hh"

#include "flint/fmpz.h"


using namespace std;


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


  // additive reduction
  // we could use FLINT for this, but composition of polynomials hangs
  auto fq_ctx = base_field_table->fq_ctx;

  vector<fq_nmod_struct*> fq_rhs = curve.rhs_coefficients();

  fq_nmod_t shift;
  fq_nmod_init(shift, fq_ctx);
  fq_nmod_set(shift, fq_rhs[curve.degree()-1], fq_ctx);

  fq_nmod_t tmp;
  fq_nmod_init(tmp, fq_ctx);

  // divide by degree
  fq_nmod_set_ui(tmp, curve.degree(), fq_ctx);
  fq_nmod_inv(tmp, tmp, fq_ctx);
  fq_nmod_mul(shift, shift, tmp, fq_ctx);

  // divide by highest coefficient
  fq_nmod_set(tmp, fq_rhs.back(), fq_ctx);
  fq_nmod_inv(tmp, tmp, fq_ctx);
  fq_nmod_mul(shift, shift, tmp, fq_ctx);

  fq_nmod_neg(shift, shift, fq_ctx);

  auto rhs_shifted = Curve( base_field_table,
                            CurveIterator::_shift_fq_polynomial(fq_rhs, shift, fq_ctx) )
                       .rhs_coeff_exponents();

  for ( auto c : fq_rhs ) {
    fq_nmod_clear(c, fq_ctx);
    delete c;
  }
  fq_nmod_clear(shift, fq_ctx);


  // multiplicative reduction via rescaling of x and y
  const auto & support = Curve::_support(base_field_table, rhs_shifted);

  // first normalize the difference between the second and third nonvanishing coefficient
  // by means of x -> (b*x)
  if ( support.size() > 2 ) {
    unsigned int snd_dx = *(support.crbegin()+1);
    unsigned int trd_dx = *(support.crbegin()+2);
    int snd_coeff = rhs_shifted[snd_dx];
    int trd_coeff = rhs_shifted[trd_dx];


    int reduce_diff = base_field_table->reduce_generator_exponent(
          base_field_table->power_coset_representative(snd_dx-trd_dx, trd_coeff-snd_coeff)
        - (trd_coeff-snd_coeff) );
    for ( auto ix : support )
      rhs_shifted[ix] = base_field_table->reduce_generator_exponent(rhs_shifted[ix] + ix*reduce_diff);
  }


  // second normalize by means of y -> (b*y)^2
  unsigned int reduction_ix;
  if ( support.size() <= 2 )
    reduction_ix = support.front(); 
  else
    reduction_ix = *(support.crbegin()+1);

  int reduce_diff =
        base_field_table->power_coset_representative(2, rhs_shifted[reduction_ix])
      - rhs_shifted[reduction_ix];

  for ( auto ix : support )
    rhs_shifted[ix] = base_field_table->reduce_generator_exponent(rhs_shifted[ix] + reduce_diff);


  return Curve(base_field_table, rhs_shifted);
}

bool
CurveIterator::
is_minimal_in_isomorphism_class(
    const Curve & curve
    )
{
  if ( !CurveIterator::is_reduced(curve) )
    return false;

  for ( unsigned int ix=0; ix<curve.prime_power()-1; ++ix )
    if ( curve > CurveIterator::reduce(CurveIterator::z_shift(curve, ix)) )
      return false;

  return true;
}

Curve
CurveIterator::
z_shift(
    const Curve & curve,
    unsigned int generator_power
    )
{
  auto base_field_table = curve.base_field_table();

  return Curve( base_field_table,
                CurveIterator::_shift_fq_polynomial(
                  curve.rhs_coefficients(),
                  base_field_table->at(generator_power),
                  base_field_table->fq_ctx ) );
}

vector<fq_nmod_struct*>
CurveIterator::
_shift_fq_polynomial(
  const vector<fq_nmod_struct*> & fq_poly,
  const fq_nmod_t shift,
  const fq_nmod_ctx_t fq_ctx
  )
{
  fmpz_t binomial;
  fmpz_init(binomial);

  fq_nmod_t shift_pw;
  fq_nmod_init(shift_pw, fq_ctx);
  fq_nmod_set(shift_pw, shift, fq_ctx);

  fq_nmod_t tmp;
  fq_nmod_init(tmp, fq_ctx);


  // contribution of shift^0
  vector<fq_nmod_struct*> fq_poly_shifted;
  fq_poly_shifted.reserve(fq_poly.size());
  for ( auto c : fq_poly ) {
    auto fq_coeff = new fq_nmod_struct;
    fq_nmod_init(fq_coeff, fq_ctx);
    fq_nmod_set(fq_coeff, c, fq_ctx);
    fq_poly_shifted.push_back(fq_coeff);
  }

  // contribution of shift^dx
  for ( unsigned int dx=1; dx<fq_poly.size(); ++dx ) {
    // from the term (x + shift)^ox
    for ( unsigned int ox=fq_poly.size()-1; ox>=dx; --ox ) {
      fmpz_bin_uiui(binomial, ox, dx);
      fq_nmod_set_ui(tmp, fmpz_get_ui(binomial) , fq_ctx);
      fq_nmod_mul(tmp, tmp, shift_pw, fq_ctx);
      fq_nmod_mul(tmp, tmp, fq_poly[ox], fq_ctx);
      fq_nmod_add(fq_poly_shifted[ox-dx], fq_poly_shifted[ox-dx], tmp, fq_ctx);
    }

    fq_nmod_mul(shift_pw, shift_pw, shift, fq_ctx);
  }


  fmpz_clear(binomial);
  fq_nmod_clear(shift_pw, fq_ctx);
  fq_nmod_clear(tmp, fq_ctx);


  return fq_poly_shifted;
}
