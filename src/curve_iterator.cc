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
              coset_product.insert(table.reduce_index(sets[ix][sx]+b));
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
    unsigned int prime,
    unsigned int prime_power,
    vector<unsigned int> coeff_support
    )
{
  unsigned int prime_power_pred = prime_power - 1;

  unsigned int degree = coeff_support.back();
  if ( prime == 2 ) {
    cerr << "CurveIterator::multiplicity: "
         << "multiplicity implemented only if prime is odd" << endl;
    throw;
  }

  if ( prime > degree ) {
    // in this case, there is no obstruction to the additive reduction x -> x + b

    if ( coeff_support.size() <= 2 )
      // the orbit of a_n x^n consists of a_n b_2^2 (x+b_1)^n
      return prime_power * prime_power_pred / 2;
    else {
      // the orbit of a_n x^n + a_m x^m + a_l x^l + ....
      // is b_2^2 ( a_n (b_3 x+b_1)^n + a_m (b_3 x+b_1)^m + a_l (b_3 x+b_1)^l + .... )
      // the orbit size comes from the (n-1)-th coefficent, the m-th coeffficent and the qutient of the
      // m-th by the l-th one
      unsigned int snd_exp = *(coeff_support.rbegin()+1);
      unsigned int trd_exp = *(coeff_support.rbegin()+2);

      return   prime_power * prime_power_pred / 2
             * prime_power_pred / n_gcd(prime_power_pred, snd_exp-trd_exp);
    }
  } else if ( degree < prime ) {
    // the orbit of a _n x^n + ...
    // is b_2^2 ( a_n (x + b_1)^n + ... )
    return prime_power * prime_power_pred / 2;
  } else if ( coeff_support.size() == 1 ||
              coeff_support.size() == 2 && coeff_support.front() == 0 ) {
    // the orbit of a_n x^n is b_2^2 a_n x^n
    // the orbit of a_n x^n + a_0 is b_2^2 (a_n x^n + a_0)
    return prime_power_pred / 2;
  } else {
    // the orbit of a_n x^n + a_l x^l + ... if l \ne 0 is
    // b_2^2 ( a_n (x + b_1)^n + a_l (x + b_1)^l + ... )
    return   prime_power * prime_power_pred / 2;
  }
}
