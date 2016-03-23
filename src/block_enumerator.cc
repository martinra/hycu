#include <set>

#include <block_enumerator.hh>


BlockEnumerator::
BlockEnumerator(
    int degree,
    const map<int,tuple<int,int>> & blocks,
    const map<int,vector<int>> & sets,
    const vector<vector<int>> couplings,
    unsigned int package_size
    ) :
  length_( length ),
{
  // note: behavior of indices that are not coupled, nor have a set of block attached is undefined

  for ( auto coupling : couplings ) {
    int attached_set_ix = -1;
    vector<int> free_coupled;
    for ( auto c : coupling ) {
      auto block_it = blocks.find(c);
      if ( block_it != blocks.end() ) {
        cerr << "coupling allows only between sets" << endl;
        throw;
      }

      auto set_it = sets.find(c);
      if ( set_it == sets.end() )
        free_coupled.push_back(c);
      else {
        if ( attached_set_ix != -1 ) {
          cerr << "each coupled set may only have one set attached to it" << endl;
          throw;
        }
        attached_set_ix = c;
      }
    }

  int dx = this->dx;
  if (dx == 0) {
    this->poly_coeffs[dx] += this->package_size;
    if (this->poly_coeffs[dx] >= this->prime) {
      this->poly_coeffs[dx] = 0;
      ++this->dx;
      return this->step();
    } else
      return *this;

  } else if (dx == this->poly_coeffs.size()) {
    if (this->poly_coeffs.size() == this->degree()) {
      this->poly_coeffs = vector<int>(this->degree()+1, 0);
      this->poly_coeffs.back() = 1;
      this->dx = 0;
      return this->step();
    } else {
      this->is_end_ = true;
      return *this;
    }

  } else if (dx == this->poly_coeffs.size() - 1) {
    if (this->prime != 2 && this->poly_coeffs.back() == 1) {
      this->poly_coeffs.back() = this->fp_non_square();
      fill(poly_coeffs.begin(), poly_coeffs.end()-1, 0);
      this->dx = 0;
      return this->step();
    } else {
      ++this->dx;
      return this->step();
    }

  } else if (dx == this->poly_coeffs.size() - 2) {
    ++this->dx;
    return this->step();

  } else {
    ++this->poly_coeffs[dx];
    if (this->poly_coeffs[dx] >= this->prime) {
      fill(poly_coeffs.begin(), poly_coeffs.begin()+dx, 0);
      ++this->dx;
      return this->step();
    } else {
      this->dx = 0;
      return *this;
    }
  }

    if ( attached_set_ix == -1 ) {
      cerr << "each coupling must have a set attached to it" << endl;
      throw;
    }

    this->couplings[attached_set_ix] = free_coupled;
  }


  vector<unsigned int> block_size;
  for (size_t ix=0; ix<size; ++ix) {
    auto blocks_it = blocks.find(ix);
    auto sets_it = sets.find(ix);

    if ( !(blocks_it != blocks.end() && sets_it != sets.end()) ) {
      cerr << "BlockEnumerator: each index must be given by a unique block or set" << endl;
      throw;
    }

    if ( sets_it != sets.end() ) {
      this->update_order_sets.push_back(ix);

      if ( sets_it->second.empty() ) {
        cerr << "BlockEnumerator: sets must have size at least 1" << endl;
        throw;
      }
    }

    if ( blocks_it != blocks.end() ) {
      int lbd, ubd;
      tie(lbd,ubd) = blocks_it->second;
      if ( lbd >= ubd ) {
        cerr << "BlockEnumerator: block lower and upper bound must differ by at least 1" << endl;
        throw;
      }

      this->blocks[ix] = make_tuple(lbd, ubd, ubd - lbd);
      this->update_order_blocks.push_back(ix);
      block_sizes.push_back(ubd - lbd);
    }
  }
  this->sets = sets;


  if ( !blocks.empty() && package_size != 1 ) {
    // we put the whole block size into one block
    vector<int> block_rests;
    for ( auto bs : block_sizes )
      block_rests.push_back( bs % package_size );
    size_t min_block_rest_ix = distance(block_rests.begin(), block_rests.min_element());

    swap( this->update_order_blocks.begin(),
          this->update_order_blocks.begin() + min_block_rest_ix );
    get<2>( this->blocks[this->update_order_blocks.front()] ) = package_size;
  }
  

  this->position = vector<int>(length);
  for ( size_t ix : this->update_order_blocks )
    this->position[ix] = get<0>(this->blocks[ix]);
  for ( size_t ix : this->update_order_sets ) {
    this->position[ix] = 0;

    auto couplings_it = this->couplings.find(ix);
    if ( couplings_it != this->couplings.end() )
      for ( size_t jx : couplings_it->second )
        this->position[jx] = 0;
  }


  if ( length > 0 )
    this->has_reached_end = false;
  else
    this->has_reached_end = true;
}

vector<tuple<int,int>>
CurveEnumerator::
as_position()
{
  auto position = this->position;

  for ( auto set_it : this->sets ) {
    size_t ix = set_it->first;

    position[ix] = set_it->second[position[ix]];
    for ( auto cx : this->couplings[ix] )
      position[cx] = position[ix];
  }

  return position;
}

vector<tuple<int,int>>
CurveEnumerator::
as_block()
{
  auto bounds = vector<tuple<int,int>>(this->length_, make_tuple(0,0));

  for ( auto block_it : this->blocks ) {
    size_t ix = block_it->first;
    int lbd = this->position[ix];

    int ubd = lbd + get<2>(blocks_it->second);
    if (ubd > get<1>(blocks_it->second))
      ubd = get<1>(blocks_it->second);

    bounds[ix] = make_tuple(lbd,ubd);
  }

  for ( auto set_it : this->sets ) {
    size_t ix = set_it->first;

    int lbd = set_it->second[this->position[ix]];
    int ubd = lbd+1;

    bounds[ix] = make_tuple(lbd,ubd);
    for ( auto cx : this->couplings[ix] )
      bounds[cx] = make_tuple(lbd,ubd);
  }

  return bounds;
}

CurveEnumerator &
CurveEnumerator::
step()
{
  if (this->has_reached_end)
    return *this;

  return this->step_(!this->blocks.empty(), 0);
}

CurveEnumerator &
CurveEnumerator::
step_(
    bool step_block_or_set,
    size_t step_ix
    )
{
  if ( step_block_or_set ) {
    px = this->update_order_blocks[step_ix];
  
    this->position[px] += get<2>(this->blocks[px]);
    if ( this->position[px] >= get<1>(this->blocks[px]) ) {
      this->position[px] = get<0>(this->blocks[px]);
  
      ++step_ix;
      if ( step_ix < this->update_order_blocks.size() )
        return this->step_(true, step_ix);
      else if ( !this->update_order_sets.empty() )
        return this->step_(false, 0);
      else {
        this->has_reached_end = true;
        return *this;
      }
    }
  }
  else {
    px = this->update_order_sets[step_ix];
    auto coupling_it = this->couplings.find(px);
    
    ++this->position[px];
    if ( coupling_it != this->couplings.end() )
      for ( cx : coupling_it->second )
        ++this->position[cx];

    if ( this->position[px] >= this->sets[px].size() ) {
      this->position[px] = 0;
      if ( coupling_it != this->couplings.end() )
        for ( cx : coupling_it->second )
          this->position[cx] = 0;

      ++step_ix;
      if ( step_ix < this->update_order_sets.size() )
        return this->size_(true, step_ix);
      else
        this->has_reached_end = true;
    }
  }

  return *this;
}

// todo: move to tables
int
CurveEnumerator::
fp_non_square()
{
  set<int> squares;
  for (int x=1; x<this->prime; ++x)
    squares.insert(x*x % this->prime);

  for (int x=1; x<this->prime; ++x)
    if (squares.find(x) == squares.end())
      return x;
}
