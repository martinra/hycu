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
#include <tuple>

#include "block_iterator.hh"
#include "fq_element_table.hh"


using std::shared_ptr;
using std::vector;
using std::tuple;


class CurveIterator
{
  public:
    CurveIterator( const FqElementTable & table, int genus, unsigned int package_size );

    CurveIterator const& step();
    bool is_end() const;

    vector<int> inline as_position() { return this->enumerator_it->as_position(); };
    vector<tuple<int,int>> inline as_block() { return this->enumerator_it->as_block(); };

    BlockIterator inline as_block_enumerator() { return this->enumerator_it->as_block_enumerator(); };

    static unsigned int multiplicity(unsigned int prime, unsigned int prime_power, vector<unsigned int> coeff_support);

  private:
    unsigned int prime;

    vector<BlockIterator> enumerators;
    vector<BlockIterator>::iterator enumerator_it;
};

#endif
