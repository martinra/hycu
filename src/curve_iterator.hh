#ifndef _H_CURVE_ITERATOR
#define _H_CURVE_ITERATOR

#include <memory>
#include <vector>
#include <tuple>

#include <block_iterator.hh>
#include <fq_element_table.hh>


using namespace std;


class CurveIterator
{
  public:
    CurveIterator( const FqElementTable & table, int genus, unsigned int package_size );

    CurveIterator & step();
    bool is_end() const;

    vector<int> inline as_position() { return this->enumerator_it->as_position(); };
    vector<tuple<int,int>> inline as_block() { return this->enumerator_it->as_block(); };
    BlockIterator inline as_block_enumerator() { return this->enumerator_it->as_block(); };

  private:
    vector<BlockIterator> enumerators;
    vector<BlockIterator>::iterator enumerator_it;
};

#endif
