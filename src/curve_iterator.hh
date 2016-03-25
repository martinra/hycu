#ifndef _H_CURVE_ITERATOR
#define _H_CURVE_ITERATOR

#include <vector>
#include <tuple>

#include <block_iterator.hh>


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
    vector<shared_ptr<BlockIterator>> block_enumerators();
    vector<shared_ptr<BlockIterator>>::iterator enumerator_it;
};

#endif
