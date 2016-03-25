#ifndef _H_ISOGENY_COUNT_STORE
#define _H_ISOGENY_COUNT_STORE

#include <map>
#include <memory>
#include <tuple>
#include <vector>

using namespace std;


// this stored hyperelliptic curves with squarefree right hand side.
class IsogenyCountStore
{
  public:
    void register_curve(const Curve & curve);

    ostream & output_legacy(ostream & stream);

  private:
    map<tuple<vector<int>,vector<int>>, int> store;

    shared_ptr<map<vector<int>,int>> legacy_ramfication_vectors;

    int to_legacy_ramification(const vector<int> & ramifications);
};

#endif
