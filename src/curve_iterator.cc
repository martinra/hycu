#include <set>

#include <curve_iterator.hh>


CurveIterator::
CurveIterator(
    const FqElementTable & table,
    int genus,
    unsigned int package_size
    )
{
  for ( int degree = 2*genus + 1; degree < 2*genus + 3; ++degree ) {
    // next to hightest coefficient is zero
    // two consecutive coefficients depend on each other
    // and vary between a square and non-square
    
    // ix is exponent of the second non-zero coefficient
    for ( int ix=degree-2; ix>=0; --ix ) {
      // jx is the exponent of the non-zero coefficient thereafter
      for ( int jx=ix-1; jx>=-1; --jx ) {
        map<size_t, tuple<int,int>> blocks;
        map<size_t, vector<int>> sets;
        map<size_t, tuple<size_t, map<int, vector<int>>>> dependent_sets;

        // highest coefficient is nonzero
        blocks[degree] = table.block_non_zero();
        for ( size_t kx=degree-1; kx>ix; --kx )
          sets[kx] = {table.zero_index()};

        // second nonzero coefficient is determined up to squaring
        sets[ix] = table.power_coset_representatives(2);
        for ( int kx=ix-1; kx>jx; --kx )
          sets[kx] = {table.zero_index()};

        // the third nonzero coefficient is coupled with the second one
        if ( jx != -1 ) {
          // its possible values depend on the difference of exponents to the
          // previous one
          auto cosets = table.power_coset_representatives(ix-jx);

          map<int, vector<int>> coset_products;
          set<int> coset_product;

          for ( size_t sx=0; sx<sets[ix].size(); ++sx ) {
            coset_product.clear();
            for ( int b : cosets )
              coset_product.insert(table.reduce_index(sets[ix][sx]+b));
            coset_products[sx] = vector<int>(coset_product.cbegin(), coset_product.cend());
          }
          dependent_sets[jx] = make_tuple(ix, coset_products);
        }

        // remaining coefficients are free to vary
        for ( int kx = jx-1; kx>=0; --kx )
          blocks[kx] = table.block_complete();

        this->enumerators.emplace_back(
            BlockIterator(degree+1, blocks, package_size, sets, dependent_sets) );
      }
    }

    { // the case of x^degree
      map<size_t, vector<int>> sets;
      sets[degree] = table.power_coset_representatives(2);
      for (int kx=degree-1; kx>=0; --kx)
        sets[kx] = {table.zero_index()};

      this->enumerators.emplace_back(
              BlockIterator(degree+1, {}, package_size, sets, {}) );
    }
  }

  this->enumerator_it = this->enumerators.begin();
}

CurveIterator &
CurveIterator::
step()
{
  if (this->is_end()) return *this;

  this->enumerator_it->step();
  if ( this->enumerator_it->is_end() )
    ++this->enumerator_it;
}

bool
CurveIterator::
is_end()
  const
{
  return ( this->enumerator_it == this->enumerators.end() );
}
