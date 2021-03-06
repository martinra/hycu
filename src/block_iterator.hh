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


#ifndef _H_BLOCK_ITERATOR
#define _H_BLOCK_ITERATOR

#include <map>
#include <memory>
#include <vector>
#include <tuple>


using std::map;
using std::vector;
using std::shared_ptr;
using std::tuple;


typedef vector<tuple<unsigned int,unsigned int>> vuu_block;
typedef shared_ptr<vuu_block> svuu_block;


class BlockIterator
{
  public:
    BlockIterator( const vector<tuple<unsigned int,unsigned int>> & bounds);
    BlockIterator(
        size_t length,
        const map<size_t, tuple<unsigned int,unsigned int>> & blocks,
        map<size_t, vector<unsigned int>> sets = {},
        map<size_t, tuple<size_t, map<unsigned int, vector<unsigned int>>>> dependent_sets = {}
        ) :
      BlockIterator( length, blocks, 1, sets, dependent_sets ) {};
    BlockIterator(
        size_t length,
        const map<size_t, tuple<unsigned int,unsigned int>> & blocks,
        unsigned int package_size,
        map<size_t, vector<unsigned int>> sets = {},
        map<size_t, tuple<size_t, map<unsigned int, vector<unsigned int>>>> dependent_sets = {}
        );

    size_t inline length() const { return length_; };

    const BlockIterator & step();
    bool inline is_end() const { return has_reached_end; };

    vector<unsigned int> as_position();
    vector<tuple<unsigned int,unsigned int>> as_block();
    BlockIterator as_block_enumerator();

  private:
    void initialize_blocks(const map<size_t, tuple<unsigned int,unsigned int>> & blocks, unsigned int package_size = 1);
    void set_initial_position();
    const BlockIterator & step_(int step_type, size_t step_ix);

    size_t length_;

    vector<size_t> update_order_blocks;
    vector<size_t> update_order_sets;
    vector<size_t> update_order_dependend_sets;

    map<size_t, tuple<unsigned int,unsigned int,unsigned int>> blocks;
    map<size_t, vector<unsigned int>> sets;
    map<size_t, tuple<size_t, map<unsigned int, vector<unsigned int>>>> dependent_sets;

    vector<unsigned int> position;
    bool has_reached_end;
};

#endif

