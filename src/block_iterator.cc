#include <algorithm>
#include <iostream>
#include <set>

#include <block_iterator.hh>


BlockIterator::
BlockIterator(
    const vector<tuple<int,int>> & bounds
    ) :
  length_( bounds.size() )
{
  map<size_t, tuple<int,int>> blocks;
  for (size_t ix=0; ix<this->length_; ++ix)
    blocks[ix] = bounds[ix];

  this->initialize_blocks(blocks, 1);

  this->set_initial_position();
}

BlockIterator::
BlockIterator(
    size_t length,
    const map<size_t, tuple<int,int>> & blocks,
    unsigned int package_size,
    map<size_t, vector<int>> sets,
    map<size_t, tuple<size_t, map<int, vector<int>>>> dependent_sets
    ) :
  length_( length )
{
  // note: behavior of indices that are not coupled, nor have a set of block attached is undefined

  this->initialize_blocks(blocks, package_size);

  this->sets = move(sets);
  for ( auto & sets_it : this->sets )
    this->update_order_sets.push_back(sets_it.first);

  // note: dependent sets may only depend on blocks and sets
  this->dependent_sets = move(dependent_sets);
  for ( auto & sets_it : this->dependent_sets )
    this->update_order_dependend_sets.push_back(sets_it.first);


  this->set_initial_position();
}

void
BlockIterator::
initialize_blocks(
    const map<size_t, tuple<int,int>> & blocks,
    unsigned int package_size
    )
{
  unsigned int min_remainder = package_size;
  size_t min_remainder_ix = 0;

  for (size_t ix=0; ix<this->length_; ++ix) {
    auto blocks_it = blocks.find(ix);

    if ( blocks_it != blocks.end() ) {
      int lbd, ubd;
      tie(lbd,ubd) = blocks_it->second;
      if ( lbd >= ubd ) {
        cerr << "BlockIterator: block lower and upper bound must differ by at least 1" << endl;
        throw;
      }

      this->blocks[ix] = make_tuple(lbd, ubd, 1);
      this->update_order_blocks.push_back(ix);

      if ( (ubd - lbd) % package_size < min_remainder ) {
        min_remainder = (ubd - lbd) % package_size;
        min_remainder_ix = this->update_order_blocks.size() - 1;
      }
    }
  }

  if ( min_remainder_ix != 0 ) {
    size_t tmp = this->update_order_blocks[min_remainder_ix];
    this->update_order_blocks[min_remainder_ix] = this->update_order_blocks.front();
    this->update_order_blocks.front() = tmp;
  }
  if ( !this->update_order_blocks.empty() )
    get<2>( this->blocks[this->update_order_blocks.front()] ) = package_size;
}

void
BlockIterator::
set_initial_position()
{
  this->position = vector<int>(this->length_);
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

vector<int>
BlockIterator::
as_position()
{
  auto position = this->position;

  for ( auto & set_it : this->sets ) {
    size_t ix = set_it.first;
    position[ix] = set_it.second[position[ix]];
  }

  for ( auto & set_it : this->dependent_sets ) {
    size_t ix = set_it.first;
    size_t jx = get<0>(set_it.second);
    auto & map_set = get<1>(set_it.second);
    
    position[ix] = map_set[position[jx]][position[ix]];
  }

  return position;
}

vector<tuple<int,int>>
BlockIterator::
as_block()
{
  auto position = this->as_position();
  vector<tuple<int,int>> bounds;
  bounds.reserve(position.size());

  for ( int c : position )
    bounds.push_back(make_tuple(c,c+1));

  for ( auto & blocks_it : this->blocks ) {
    size_t ix = blocks_it.first;
    int lbd = get<0>(bounds[ix]);

    int ubd = lbd + get<2>(blocks_it.second);
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

  auto blocks = map<size_t, tuple<int,int>>();
  auto sets = map<size_t, vector<int>>();

  for ( auto & blocks_it : this->blocks ) {
    int lbd = position[blocks_it.first];

    int ubd = lbd + get<2>(blocks_it.second);
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

BlockIterator &
BlockIterator::
step()
{
  if (this->has_reached_end)
    return *this;

  return this->step_(!this->blocks.empty(), 0);
}

BlockIterator &
BlockIterator::
step_(
    int step_type,
    size_t step_ix
    )
{
  // stepping a block
  if ( step_type == 0 ) {
    size_t px = this->update_order_blocks[step_ix];
  
    this->position[px] += get<2>(this->blocks[px]);
    if ( this->position[px] >= get<1>(this->blocks[px]) ) {
      this->position[px] = get<0>(this->blocks[px]);
  
      ++step_ix;
      if ( step_ix < this->update_order_blocks.size() )
        return this->step_(0, step_ix);
      else if ( !this->update_order_sets.empty() )
        return this->step_(1, 0);
      else if ( !this->update_order_dependend_sets.empty() )
        return this->step_(2, 0);
      else {
        this->has_reached_end = true;
      }
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
      else if ( !this->update_order_dependend_sets.empty() )
        return this->step_(2, 0);
      else
        this->has_reached_end = true;
    }
  }
  // stepping a dependent set
  else {
    size_t px = this->update_order_dependend_sets[step_ix];
    
    ++this->position[px];
    auto & px_set = get<1>(this->dependent_sets[px]);
    if ( this->position[px] >= px_set[this->position[get<0>(this->dependent_sets[px])]].size() ) {
      this->position[px] = 0;

      ++step_ix;
      if ( step_ix < this->update_order_dependend_sets.size() )
        return this->step_(2, 0);
      else
        this->has_reached_end = true;
    }
  }

  return *this;
}
