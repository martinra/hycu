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


#include <cmath>

#include "fq_element_table.hh"


using namespace std;


FqElementTable::
FqElementTable(
    unsigned int prime,
    unsigned int prime_exponent
    ) :
  prime( prime ),
  prime_exponent( prime_exponent ),
  prime_power ( pow(prime, prime_exponent) ),
  prime_power_pred ( prime_power - 1 )
{
  this->fq_generator_powers.resize(this->prime_power);
  this->fq_elements.reserve(this->prime_power);


  fmpz_t prime_fmpz;
  fmpz_init_set_ui(prime_fmpz, prime);
  fq_nmod_ctx_init(this->_fq_nmod_ctx, prime_fmpz, prime_exponent, ((string)"T").c_str());
  fmpz_clear(prime_fmpz);
  const auto fq_ctx = this->_fq_nmod_ctx;

  fq_nmod_t gen;
  fq_nmod_init(gen, fq_ctx);
  fq_nmod_gen(gen, fq_ctx);

  fq_nmod_t genp;
  fq_nmod_init(genp, fq_ctx);

  fq_nmod_t a;
  fq_nmod_init(a, fq_ctx);
  fq_nmod_one(a, fq_ctx);

  fq_nmod_struct * b;


  for (unsigned int ix=0; ix<this->prime_power_pred; ++ix) {
    this->fq_generator_powers[FqElementTable::fq_as_index(a)] = ix;

    b = new fq_nmod_struct;
    fq_nmod_init(b, fq_ctx);
    fq_nmod_set(b, a, fq_ctx);
    fq_nmod_reduce(b, fq_ctx);
    this->fq_elements.push_back(b);

    fq_nmod_mul(a, a, gen, fq_ctx);
  }

  { // the case of b == 0
    this->fq_generator_powers[0] = this->prime_power_pred;

    b = new fq_nmod_struct;
    fq_nmod_init(b, fq_ctx);
    fq_nmod_zero(b, fq_ctx);
    fq_nmod_reduce(b, fq_ctx);
    this->fq_elements.push_back(b);
  }


  fq_nmod_clear(gen, fq_ctx);
  fq_nmod_clear(genp, fq_ctx);
  fq_nmod_clear(a, fq_ctx);
}

FqElementTable::
~FqElementTable()
{
  for ( auto fq : this->fq_elements )
    fq_nmod_clear(fq, this->_fq_nmod_ctx);

  fq_nmod_ctx_clear(this->_fq_nmod_ctx);
}


vector<int>
FqElementTable::
power_coset_representatives(
    unsigned int n
    )
  const
{
  n = n_gcd(n, this->prime_power_pred);

  vector<int> cosets;
  cosets.reserve(n);
  for (int i=0; i<n; ++i)
    cosets.push_back(i);

  return cosets;
}
