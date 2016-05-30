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


#ifndef _H_FQ_ELEMENT_TABLE
#define _H_FQ_ELEMENT_TABLE

#include <iostream>
#include <tuple>
#include <vector>
#include <flint/fq_nmod.h>


using std::tuple;
using std::make_tuple;
using std::vector;
using std::ostream;


class Curve;


class FqElementTable
{
  public:
    FqElementTable( unsigned int prime, unsigned int prime_exponent );
    ~FqElementTable();

    bool inline is_prime_field() const { return this->prime_exponent == 1; };


    inline unsigned int at_nmod(int ix) const
    {
      return nmod_poly_get_coeff_ui(this->fq_elements.at(ix), 0);
    };
    inline const fq_nmod_struct* at(int ix) const { return this->fq_elements[ix]; };
    inline const fq_nmod_struct* operator[](int ix) const { return this->fq_elements[ix]; };

    inline unsigned int generator_power(unsigned int a) const
    {
      return this->fq_generator_powers[a];
    };
    inline unsigned int generator_power(const fq_nmod_struct * a) const
    {
      return this->fq_generator_powers[FqElementTable::fq_as_index(a)];
    };


    int inline zero_index() const { return this->prime_power_pred; };
    inline tuple<int,int> block_non_zero() const { return make_tuple(0, (int)this->prime_power_pred); };
    inline tuple<int,int> block_complete() const { return make_tuple(0, (int)this->prime_power); };
    vector<int> power_coset_representatives(unsigned int n) const;

    inline bool is_zero(int a) const
    {
      return a == this->prime_power_pred;
    };
    inline bool is_power_coset_representative(unsigned int n, int a) const
    {
      return a >=0 && a < n_gcd(n, this->prime_power_pred);
    };


    unsigned int inline reduce_index(unsigned int ix) const
    {
      return ix % this->prime_power_pred;
    };

    static inline unsigned int fq_as_index(const fq_nmod_struct * a)
    {
      unsigned int ax = 0;
      for ( int dx=a->length-1; dx>=0; --dx ) {
        ax *= a->mod.n;
        ax += nmod_poly_get_coeff_ui(a,dx);
      }
    return ax;
    };

    friend Curve;
    friend class CurveIterator;
    friend ostream& operator<<(ostream & stream, const Curve & curve);

  protected:
    const unsigned int prime;
    const unsigned int prime_exponent;
    unsigned int prime_power;
    unsigned int prime_power_pred;

  private:
    fq_nmod_ctx_t fq_ctx;
    vector<unsigned int> fq_generator_powers;
    vector<fq_nmod_struct*> fq_elements;
};

#endif
