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
    unsigned int genus,
    bool with_marked_point,
    unsigned int package_size
    ) :
  prime ( table.prime )
{
  if ( prime == 2 ) {
    cerr << "CurveIterator: "
         << "implemented only if prime is odd" << endl;
    throw;
  }

  if ( genus == 0 ) {
    cerr << "CurveIterator: "
         << "implemented only if genus is positive" << endl;
    throw;
  }
    

  for ( unsigned int degree = 2*genus + 1; degree < with_marked_point ? 2*genus + 2 : 2*genus + 3; ++degree ) {
    if ( prime > degree ) {
      // next to hightest coefficient is zero
      // two consecutive coefficients depend on each other
      // and vary between a square and non-square
      
      // ix is exponent of the second non-zero coefficient
      for ( int ix=degree-2; ix>=0; --ix ) {
        // jx is the exponent of the non-zero coefficient thereafter
        for ( int jx=ix-1; jx>=-1; --jx ) {
          map<size_t, tuple<unsigned int,unsigned int>> blocks;
          map<size_t, vector<unsigned int>> sets;
          map<size_t, tuple<size_t, map<unsigned int, vector<unsigned int>>>> dependent_sets;
  
          // highest coefficient is nonzero
          blocks[degree] = table.block_non_zero();
          for ( int kx = degree-1; kx>ix; --kx )
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
  
            map<unsigned int, vector<unsigned int>> coset_products;
            set<unsigned int> coset_product;
  
            for ( size_t sx=0; sx<sets[ix].size(); ++sx ) {
              coset_product.clear();
              for ( unsigned int b : cosets )
                coset_product.insert(table.reduce_index(sets[ix][sx]+b));
              coset_products[sx] = vector<unsigned int>(coset_product.cbegin(), coset_product.cend());
            }
            dependent_sets[jx] = make_tuple((unsigned int)ix, coset_products);
          }
  
          // remaining coefficients are free to vary
          for ( int kx = jx-1; kx>=0; --kx )
            blocks[kx] = table.block_complete();
  
          this->enumerators.emplace_back(
              degree+1, blocks, package_size, sets, dependent_sets );
        }
      }
  
      { // the case of x^degree
        map<size_t, vector<unsigned int>> sets;
        // first coefficient is determined up to squaring
        // it suffices, however, to choose a square here, since the other
        // curves are twists, which we do not need enumerate separately
        sets[degree] = table.power_coset_representatives(1);
        for (int kx=degree-1; kx>=0; --kx)
          sets[kx] = {table.zero_index()};
  
        this->enumerators.push_back(
                BlockIterator(degree+1, {}, package_size, sets, {}) );
      }
    } else if ( degree % prime == 0 ) {

      // We reduce the leading coefficient by multiplying with b_2^2.
      // Since we do not enumerate quadratic twists, we fix the leading
      // coefficient.

      { // Case: Next to leading coefficient is nonzero.
        // Its exponent is not divisible by prime so that
        // we can reduce the next coefficient by x -> x + b_0.
        // The quotient of the one that follows and the leading one is
        // determined up to a cube class.
        map<size_t, tuple<unsigned int,unsigned int>> blocks;
        map<size_t, vector<unsigned int>> sets;

        sets[degree] = table.power_coset_representatives(1);
        blocks[degree-1] = table.block_non_zero();
        sets[degree-2] = {table.zero_index()};

        // assert(sets[degree][0] == 0); otherwise, we need to shift
        sets[degree-3] = table.power_coset_representatives(3);
        sets[degree-3].push_back(table.zero_index());

        for ( int kx=degree-4; kx>=0; --kx )
          blocks[kx] = table.block_complete();

        this->enumerators.push_back(
            BlockIterator(degree+1, blocks, package_size, sets, {}) );
      }

      { // Case: Next to leading coefficient is zero.
        // The third highest coefficient is determined up to a square class.
        map<size_t, tuple<unsigned int,unsigned int>> blocks;
        map<size_t, vector<unsigned int>> sets;

        sets[degree] = table.power_coset_representatives(1);
        sets[degree-1] = {table.zero_index()};

        // assert(sets[degree][0] == 0); otherwise, we need to shift
        sets[degree-2] = table.power_coset_representatives(2);
        sets[degree-2].push_back(table.zero_index());

        for ( int kx=(int)degree-3; kx>=0; --kx )
          blocks[kx] = table.block_complete();

        this->enumerators.push_back(
            BlockIterator(degree+1, blocks, package_size, sets, {}) );
      }
    } else { // prime < degree and prime does not divide degree
      // We can use the leading coefficient to reduce the next to leading one
      // by x -> x + b_0. The one that follows is determined up to a square
      // class.
      map<size_t, vector<unsigned int>> sets;
      map<size_t, tuple<unsigned int,unsigned int>> blocks;
  
      // Since we do not enumerate quadratic twists, we fix the leading
      // coefficient.
      sets[degree] = table.power_coset_representatives(1);
  
      sets[degree-1] = {table.zero_index()};

      sets[degree-2] = table.power_coset_representatives(2);
      sets[degree-2].push_back(table.zero_index());

      for ( int kx=degree-3; kx>=0; --kx )
        blocks[kx] = table.block_complete();
  
      this->enumerators.push_back(
          BlockIterator(degree+1, blocks, package_size, sets, {}) );
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

void
CurveIterator::
multiplicity(
    fmpz_t mult,
    unsigned int prime,
    unsigned int prime_power,
    vector<unsigned int> coeff_support
    )
{
  unsigned int prime_power_pred = prime_power - 1;
  unsigned int degree = coeff_support.back();

  if ( prime > degree ) {
    // in this case, there is no obstruction to the additive reduction x -> x + b

    if ( coeff_support.size() <= 2 ) {
      // the orbit of a_n x^n consists of a_n b_2^2 (x+b_1)^n
      fmpz_set_ui(mult, prime_power);
      fmpz_mul_ui(mult, mult, prime_power_pred);
      fmpz_fdiv_q_2exp(mult, mult, 1); // / 2
      return;
    } else {
      // the orbit of a_n x^n + a_m x^m + a_l x^l + ....
      // is b_2^2 ( a_n (b_3 x+b_1)^n + a_m (b_3 x+b_1)^m + a_l (b_3 x+b_1)^l + .... )
      // the orbit size comes from the (n-1)-th coefficent, the m-th coeffficent and the qutient of the
      // m-th by the l-th one
      unsigned int snd_exp = *(coeff_support.rbegin()+1);
      unsigned int trd_exp = *(coeff_support.rbegin()+2);

      fmpz_set_ui(mult, prime_power);
      fmpz_mul_ui(mult, mult, prime_power_pred);
      fmpz_fdiv_q_2exp(mult, mult, 1); // / 2
      fmpz_mul_ui(mult, mult, prime_power_pred);
      fmpz_divexact_ui(mult, mult, n_gcd(prime_power_pred, snd_exp-trd_exp));
      return;
    }
  } else if ( degree % prime == 0 ) {
    if ( coeff_support.size() >= 2 &&
         *(coeff_support.rbegin()+1) == degree-1 ) {
      if ( coeff_support.size() >= 3 &&
           *(coeff_support.rbegin()+2) == degree-3 ) {
        // the orbit of a_n x^n + a_{n-1} x^{n-1} + ... is
        // b_2^2 ( a_n (b_3 x + b_1)^n + a_{n-1} (b_3 x + b_1)^{n-1} + ... )
        // (using that prime is odd)
        fmpz_set_ui(mult, prime_power);
        fmpz_mul_ui(mult, mult, prime_power_pred);
        fmpz_fdiv_q_2exp(mult, mult, 1); /// / 2
        fmpz_mul_ui(mult, mult, prime_power_pred);
        fmpz_divexact_ui(mult, mult, n_gcd(prime_power_pred, 3));
        return;
      } else {
        // the orbit of a_n x^n + a_{n-1} x^{n-1} + ... is
        // b_2^2 ( a_n (x + b_1)^n + a_{n-1} (x + b_1)^{n-1} + ... )
        // (using that prime is odd)
        fmpz_set_ui(mult, prime_power);
        fmpz_mul_ui(mult, mult, prime_power_pred);
        fmpz_fdiv_q_2exp(mult, mult, 1); // / 2
        return;
      }
    } else {
      if ( coeff_support.size() >= 2 &&
           *(coeff_support.rbegin()+1) == degree-2 ) {
        // the orbit of a_n x^n + a_{n-2} x^{n-2} + ... is
        // b_2^2 ( a_n (b_3 x)^n + a_{n-2} (b_3 x)^{n-2} + ... )
        fmpz_set_ui(mult, prime_power_pred);
        fmpz_mul_ui(mult, mult, prime_power_pred);
        fmpz_fdiv_q_2exp(mult, mult, 2); // / 4
        return;
      } else {
        // the orbit of a_n x^n + a_{n-3} x^{n-3} + ... is
        // b_2^2 ( a_n x^n + a_{n-3} x^{n-3} + ... )
        fmpz_set_ui(mult, prime_power_pred);
        fmpz_fdiv_q_2exp(mult, mult, 1); // / 2
        return;
      }
    }
  } else { // degree % prime != 0
    if ( coeff_support.size() >= 2 &&
         *(coeff_support.rbegin()+1) == degree-2 ) {
      // the orbit of a _n x^n + ...
      // is b_2^2 ( a_n (b_3 x + b_1)^n + ... )
      fmpz_set_ui(mult, prime_power);
      fmpz_mul_ui(mult, mult, prime_power_pred);
      fmpz_mul_ui(mult, mult, prime_power_pred);
      fmpz_fdiv_q_2exp(mult, mult, 2); // / 4
      return;
    } else {
      // the orbit of a _n x^n + ...
      // is b_2^2 ( a_n (x + b_1)^n + ... )
      fmpz_set_ui(mult, prime_power);
      fmpz_mul_ui(mult, mult, prime_power_pred);
      fmpz_fdiv_q_2exp(mult, mult, 1); // / 2
      return;
    }
  }
}
