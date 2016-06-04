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


#ifndef _H_CURVE_ITERATOR
#define _H_CURVE_ITERATOR

#include <memory>
#include <vector>
#include <set>
#include <tuple>

#include "flint/fq_nmod.h"


#include "block_iterator.hh"
#include "curve.hh"
#include "fq_element_table.hh"


using std::shared_ptr;
using std::vector;
using std::set;
using std::tuple;


class CurveIterator
{
  public:
    CurveIterator( const FqElementTable & table, int genus, unsigned int package_size );

    CurveIterator const& step();
    bool is_end() const;

    vector<int> inline as_position() { return this->enumerator_it->as_position(); };
    vector<tuple<int,int>> inline as_block() { return this->enumerator_it->as_block(); };
    BlockIterator inline as_block_enumerator() { return this->enumerator_it->as_block(); };


    static unsigned int multiplicity(const Curve & curve);

    static bool is_reduced(const Curve & curve);
    static Curve reduce(const Curve & curve);

    static
    inline
    Curve
    reduce_multiplicative(
        shared_ptr<FqElementTable> base_field_table,
        vector<fq_nmod_struct*> && poly_coeff_exponents
        )
    {
      return CurveIterator::reduce_multiplicative(
          base_field_table,
          Curve(base_field_table, move(poly_coeff_exponents)).rhs_coeff_exponents() );
    };

    static Curve reduce_multiplicative(shared_ptr<FqElementTable> base_field_table, vector<int> && poly_coeff_exponents);

    static set<vector<int>> orbit(const Curve & curve, map<vector<int>, unsigned int> orbits);

  private:
    unsigned int prime;

    vector<BlockIterator> enumerators;
    vector<BlockIterator>::iterator enumerator_it;


    static
    inline
    Curve
    x_shift(
        const Curve & curve,
        unsigned int generator_power
        )
    {
      return Curve( curve.base_field_table(),
                    CurveIterator::x_shift(curve.base_field_table(), curve.rhs_coefficients(), generator_power) );
    };

    inline
    static
    Curve
    x_shift(
        const Curve & curve,
        const fq_nmod_t shift
        )
    {
      return Curve( curve.base_field_table(),
                    CurveIterator::x_shift(curve.base_field_table(), curve.rhs_coefficients(), shift) );
    };

    static
    inline
    vector<fq_nmod_struct*>
    x_shift(
        const shared_ptr<FqElementTable> base_field_table,
        const vector<fq_nmod_struct*> & poly,
        unsigned int generator_power
        )
    {
      return CurveIterator::x_shift(base_field_table, poly, base_field_table->at(generator_power));

    };

    static
    inline
    vector<fq_nmod_struct*>
    x_shift(
        const shared_ptr<FqElementTable> base_field_table,
        const vector<fq_nmod_struct*> & poly,
        const fq_nmod_t shift
        )
    {
      return CurveIterator::_shift_polynomial(poly, shift, base_field_table->fq_ctx);
    };


    static
    inline
    Curve
    z_shift(
        const Curve & curve,
        unsigned int generator_power
        )
    {
      return Curve( curve.base_field_table(),
                    CurveIterator::z_shift(curve.base_field_table(), curve.rhs_coefficients(), generator_power) );
    };

    inline
    static
    Curve
    z_shift(
        const Curve & curve,
        const fq_nmod_t shift
        )
    {
      return Curve( curve.base_field_table(),
                    CurveIterator::z_shift(curve.base_field_table(), curve.rhs_coefficients(), shift) );
    };

    static
    inline
    vector<fq_nmod_struct*>
    z_shift(
        const shared_ptr<FqElementTable> base_field_table,
        const vector<fq_nmod_struct*> & poly,
        unsigned int generator_power
        )
    {
      return CurveIterator::z_shift(base_field_table, poly, base_field_table->at(generator_power));

    };

    static vector<fq_nmod_struct*> z_shift(const shared_ptr<FqElementTable> base_field_table, const vector<fq_nmod_struct*> & poly, const fq_nmod_t shift);

    static vector<fq_nmod_struct*> _shift_polynomial(const vector<fq_nmod_struct*> & fq_poly, const fq_nmod_t shift, const fq_nmod_ctx_t fq_ctx);
};

#endif
