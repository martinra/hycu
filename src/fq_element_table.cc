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
  prime_exponent( prime_exponent )
{
  this->prime_power = pow(prime, prime_exponent);
  this->prime_power_pred = this->prime_power - 1;

  fmpz_t prime_fmpz;
  fmpz_init_set_ui(prime_fmpz, prime);
  fq_nmod_ctx_init(this->fq_ctx, prime_fmpz, prime_exponent, ((string)"T").c_str());
  fmpz_clear(prime_fmpz);

  fq_nmod_t gen;
  fq_nmod_init(gen, this->fq_ctx);
  fq_nmod_gen(gen, this->fq_ctx);

  fq_nmod_t a;
  fq_nmod_init(a, this->fq_ctx);
  fq_nmod_one(a, this->fq_ctx);

  this->fq_elements.reserve(this->prime_power);
  for (size_t ix=0; ix<this->prime_power_pred; ++ix) {
    auto b = new fq_nmod_struct;
    fq_nmod_init(b, this->fq_ctx);
    fq_nmod_set(b, a, this->fq_ctx);
    fq_nmod_reduce(b, this->fq_ctx);
    this->fq_elements.push_back(b);

    fq_nmod_mul(a, a, gen, this->fq_ctx);
  }

  { // the case of b == 0
    auto b = new fq_nmod_struct;
    fq_nmod_init(b, this->fq_ctx);
    fq_nmod_zero(b, this->fq_ctx);
    fq_nmod_reduce(b, this->fq_ctx);
    this->fq_elements.push_back(b);
  }

  fq_nmod_clear(gen, this->fq_ctx);
  fq_nmod_clear(a, this->fq_ctx);
}

FqElementTable::
~FqElementTable()
{
  for ( auto fq : this->fq_elements )
    fq_nmod_clear(fq, this->fq_ctx);

  fq_nmod_ctx_clear(this->fq_ctx);
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
