#ifndef _H_BLOCK_ITERATOR
#define _H_BLOCK_ITERATOR

#include <vector>
#include <tuple>


using namespace std;


class BlockIterator
{
  public:
    BlockIterator( const vector<tuple<int,int>> & bounds);
    BlockIterator( size_t length,
                     const map<size_t, tuple<int,int>> & blocks,
                     unsigned int package_size = 1,
                     auto sets = map<size_t, vector<int>>(),
                     auto dependent_sets = map<size_t, tuple<size_t, map<int, vector<int>>>>(),
                     );

    size_t inline length() const { return length_; };

    BlockIterator & step();
    bool inline is_end() const { return has_reached_end; };

    vector<int> as_position();
    vector<tuple<int,int>> as_block();
    BlockIterator as_block_enumerator();

  private:
    void initialize_blocks(const vector<tuple<int,int>> & block, unsigned int package_size);
    void set_initial_position();
    BlockIterator & step_(int step_type, size_t step_ix);

    const size_t length_;

    vector<size_t> update_order_blocks;
    vector<size_t> update_order_sets;
    vector<size_t> update_order_dependend_sets;

    map<size_t, tuple<int,int,unsigned int>> blocks;
    map<size_t, vector<int>> sets;
    map<size_t, tuple<size_t, map<int, vector<int>>>> dependent_sets,

    vector<int> position;
    bool has_reached_end;
};

#endif

