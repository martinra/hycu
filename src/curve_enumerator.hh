#ifndef _H_CURVE_ENUMERATOR
#define _H_CURVE_ENUMERATOR

#include <vector>
#include <tuple>

#include <block_enumerator.hh>


using namespace std;


class CurveEnumerator
{
  public:
    BlockEnumerator( const EnumerationTable & table, int genus, unsigned int package_size );

    CurveEnumerator & step();
    bool at_end() const;

    vector<int> inline as_position() { return this->enumerator_it->as_position(); };
    vector<tuple<int,int>> inline as_block() { return this->enumerator_it->as_block(); };
    BlockEnumerator inline as_block_enumerator() { return this->enumerator_it->as_block(); };

  private:
    vector<shared_ptr<BlockEnumerator>> block_enumerators();
    vector<shared_ptr<BlockEnumerator>>::iterator enumerator_it;
};

#endif
