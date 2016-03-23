#ifndef _H_BLOCK_ENUMERATOR
#define _H_BLOCK_ENUMERATOR

#include <vector>
#include <tuple>


using namespace std;


class BlockEnumerator
{
  public:
    // BlockEnumerator(int prime, int degree, int block_size);
    BlockEnumerator( int length,
                     const map<int, tuple<int,int>> & blocks,
                     const map<int, vector<int>> & sets,
                     const vector<vector<int>> couplings,
                     unsigned int package_size );

    int inline length() const { return length_; };

    bool inline valid_position() const { return has_reached_end; };
    vector<int> as_position();
    vector<tuple<int,int>> as_block();

    BlockEnumerator & step();

  private:
    BlockEnumerator & step_(bool step_block_or_set, size_t step_ix);

    const int length_;

    vector<int> update_order_blocks;
    vector<int> update_order_sets;
    map<int, vector<int>> couplings;

    map<int, tuple<int,int,unsigned int>> blocks;
    map<int, vector<int>> sets;

    vector<int> position;
    bool has_reached_end;
};

#endif

