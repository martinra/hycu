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


#include <algorithm>
#include <iostream>
#include <set>

#include "block_iterator.hh"


using namespace std;


BlockIterator::
BlockIterator(
    const vector<tuple<unsigned int,unsigned int>> & bounds
    ) :
  length_( bounds.size() )
{
  map<size_t, tuple<unsigned int,unsigned int>> blocks;
  for (size_t ix=0; ix<this->length_; ++ix)
    blocks[ix] = bounds[ix];

  this->initialize_blocks(blocks, 1);

  this->set_initial_position();
}

BlockIterator::
BlockIterator(
    size_t length,
    const map<size_t, tuple<unsigned int,unsigned int>> & blocks,
    unsigned int package_size,
    map<size_t, vector<unsigned int>> sets,
    map<size_t, tuple<size_t, map<unsigned int, vector<unsigned int>>>> dependent_sets
    ) :
  length_( length )
{
  // note: behavior of indices that are not coupled, nor have a set of block attached is undefined
  // note: none of the blocks, sets, and dependent sets may be empty

  this->initialize_blocks(blocks, package_size);

  this->sets = move(sets);
  for ( auto & sets_it : this->sets )
    this->update_order_sets.push_back(sets_it.first);

  // note: dependent sets may only depend on blocks (if package_size==1) and sets
  this->dependent_sets = move(dependent_sets);
  for ( auto & sets_it : this->dependent_sets )
    this->update_order_dependend_sets.push_back(sets_it.first);

  this->set_initial_position();
}

void
BlockIterator::
initialize_blocks(
    const map<size_t, tuple<unsigned int,unsigned int>> & blocks,
    unsigned int package_size
    )
{
  vector<tuple<size_t,unsigned int>> block_sizes;

  for ( auto & blocks_it : blocks ) {
    unsigned int lbd, ubd;
    tie(lbd,ubd) = blocks_it.second;

    if ( lbd >= ubd ) {
      cerr << "BlockIterator: block lower and upper bound must differ by at least 1" << endl;
      throw;
    }

    block_sizes.push_back(make_tuple(ubd-lbd,blocks_it.first));
  }
  sort( block_sizes.rbegin(), block_sizes.rend() );
  

  for ( auto & block_size_it : block_sizes ) {
    unsigned int block_size = get<0>(block_size_it);
    size_t ix = get<1>(block_size_it);

    unsigned int step_size = package_size >= block_size ? block_size : package_size;
    package_size /= step_size;

    this->update_order_blocks.push_back(ix);
    unsigned int lbd, ubd; tie(lbd,ubd) = blocks.at(ix);
    this->blocks[ix] = make_tuple(lbd, ubd, step_size);
  }
}

void
BlockIterator::
set_initial_position()
{
  this->position = vector<unsigned int>(this->length_);
  for ( size_t ix : this->update_order_blocks )
    this->position[ix] = get<0>(this->blocks[ix]);
  for ( size_t ix : this->update_order_sets )
    this->position[ix] = 0;
  for ( size_t ix : this->update_order_dependend_sets )
    this->position[ix] = 0;


  if ( this->length_ > 0 )
    this->has_reached_end = false;
  else
    this->has_reached_end = true;
}

vector<unsigned int>
BlockIterator::
as_position()
{
  auto position = this->position;

  for ( auto & set_it : this->dependent_sets ) {
    size_t ix = set_it.first;
    size_t jx = get<0>(set_it.second);
    auto & map_set = get<1>(set_it.second);
    
    position[ix] = map_set[position[jx]][position[ix]];
  }

  for ( auto & set_it : this->sets ) {
    size_t ix = set_it.first;
    position[ix] = set_it.second[position[ix]];
  }

  return position;
}

vector<tuple<unsigned int,unsigned int>>
BlockIterator::
as_block()
{
  auto position = this->as_position();
  vector<tuple<unsigned int,unsigned int>> bounds;
  bounds.reserve(position.size());

  for ( unsigned int c : position )
    bounds.push_back(make_tuple(c,c+1));

  for ( auto & blocks_it : this->blocks ) {
    size_t ix = blocks_it.first;
    unsigned int lbd = get<0>(bounds[ix]);

    unsigned int ubd = lbd + get<2>(blocks_it.second);
    if (ubd > get<1>(blocks_it.second))
      ubd = get<1>(blocks_it.second);

    bounds[ix] = make_tuple(lbd,ubd);
  }

  return bounds;
}

BlockIterator
BlockIterator::
as_block_enumerator()
{
  auto position = this->as_position();

  auto blocks = map<size_t, tuple<unsigned int,unsigned int>>();
  auto sets = map<size_t, vector<unsigned int>>();

  for ( auto & blocks_it : this->blocks ) {
    unsigned int lbd = position[blocks_it.first];

    unsigned int ubd = lbd + get<2>(blocks_it.second);
    if (ubd > get<1>(blocks_it.second))
      ubd = get<1>(blocks_it.second);

    if (ubd - lbd > 1)
      blocks[blocks_it.first] = make_tuple(lbd,ubd);
    else
      sets[blocks_it.first] = {lbd};
  }

  for ( auto & sets_it : this->sets )
    sets[sets_it.first] = {position[sets_it.first]};
  for ( auto & sets_it : this->dependent_sets )
    sets[sets_it.first] = {position[sets_it.first]};

  return BlockIterator(this->length(), blocks, sets);
}

const BlockIterator &
BlockIterator::
step()
{
  if (this->has_reached_end)
    return *this;

  int step_type;
  if ( !this->dependent_sets.empty() )
    step_type = 0;
  else if ( !this->sets.empty() )
    step_type = 1;
  else
    step_type = 2;

  return this->step_(step_type, 0);
}

const BlockIterator &
BlockIterator::
step_(
    int step_type,
    size_t step_ix
    )
{
  // stepping a dependent set
  if ( step_type == 0 ){
    size_t px = this->update_order_dependend_sets[step_ix];

    ++this->position[px];
    auto & px_set = get<1>(this->dependent_sets[px]);
    size_t coupled_px = get<0>(this->dependent_sets[px]);
    if ( this->position[px] >= px_set[this->position[coupled_px]].size() ) {
      this->position[px] = 0;

      ++step_ix;
      if ( step_ix < this->update_order_dependend_sets.size() )
        return this->step_(0, step_ix);
      else if ( !this->update_order_sets.empty() )
        return this->step_(1, 0);
      else if ( !this->update_order_blocks.empty() )
        return this->step_(2, 0);
      else
        this->has_reached_end = true;
    }
  }

  // stepping a set
  else if ( step_type == 1 ){
    size_t px = this->update_order_sets[step_ix];
    
    ++this->position[px];
    if ( this->position[px] >= this->sets[px].size() ) {
      this->position[px] = 0;

      ++step_ix;
      if ( step_ix < this->update_order_sets.size() )
        return this->step_(1, step_ix);
      else if ( !this->update_order_blocks.empty() )
        return this->step_(2, 0);
      else
        this->has_reached_end = true;
    }
  }

  // stepping a block
  else { // if ( step_type == 2 )
    size_t px = this->update_order_blocks[step_ix];

    this->position[px] += get<2>(this->blocks[px]);
    if ( this->position[px] >= get<1>(this->blocks[px]) ) {
      this->position[px] = get<0>(this->blocks[px]);

      ++step_ix;
      if ( step_ix < this->update_order_blocks.size() )
        return this->step_(2, step_ix);
      else {
        this->has_reached_end = true;
      }
    }
  }

  return *this;
}
