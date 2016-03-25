#include <curve_enumerator.hh>


CurveEnumerator::
CurveEnumerator(
    const EnumerationTable & table,
    int genus,
    unsigned int package_size
    )
{
  for ( int degree = 2*genus + 1; degree < 2*genus + 3; ++degree ) {
    // next to hightest coefficient is zero
    // two consecutive coefficients are the same
    // and vary between a square and non-square
    
    // ix is exponent of the second non-zero coefficient
    for (size_t ix=degree-2; ix>0; --ix) {
      // jx is the exponent of the non-zero coefficient thereafter
      for (size_t jx=ix-1; jx>=-1; --jx) {
        map<size_t, tuple<int,int>> blocks;
        map<size_t, vector<int>> sets;
        map<size_t, tuple<size_t, map<int, vector<int>>>> dependent_sets;

        // highest coefficient is non-zero
        // todo: implement non-zero-bounds
        blocks[degree] = table.block_non_zero();
        // next to highest coefficient is zero
        // todo: implement zero-set
        sets[degree-1] = {table.zero_index()};

        // where the iteration set of one index depends on the next one
        sets[ix] = table.power_coset_representatives(2);


        if (jx != -1) {
          auto cosets = table.power_coset_representatives(ix-jx);

          map<int, vector<int>> coset_products;
          vector<int> coset_product;
          coset_product.reserve(cosets.size());

          for ( int a : sets[ix] ) {
            for ( int b : cosets )
              coset_product.push_back(table.reduce_index(a+b));
            coset_products[ix] = coset_product;
          }
          dependent_sets[jx] = make_tuple(ix,coset_products);
        }

        for (size_t kx = jx-1; kx>=0; --kx)
          blocks[kx] = table.block_complete();

        this->enumerators.emplace_back(
            BlockEnumerator(degree+1, blocks, package_size, sets, dependent_sets) );
      }
    }
  }
}

CurveEnumerator &
CurveEnumerator::
step()
{
  if (this->at_end()) return *this;

  this->enumerator_it->step();
  if ( this->enumerator_it->at_end() )
    ++this->enumerator_it;
}

bool
CurveEnumerator::
at_end()
  const
{
  return this->enumerator_it = this->enumerators.end();
}
