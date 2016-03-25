#ifndef _H_ISOGENY_COUNT_STORE
#define _H_ISOGENY_COUNT_STORE

#include <map>
#include <memory>
#include <tuple>
#include <vector>
#include <flint/nmod_vec.h>
#include <flint/nmod_poly.h>

using namespace std;


// this stored hyperelliptic curves with squarefree right hand side.
class IsogenyCountStore
{
  public:
    IsogenyCountStore(int prime);
    ~IsogenyCountStore();

    void register_poly(const vector<int> & poly_coeffs, const vector<tuple<int,int>> & nmb_points);

    ostream & output_legacy(ostream & stream);

  private:
    int to_legacy_ramification(const vector<int> & ramifications);
    shared_ptr<map<vector<int>,int>> legacy_ramfication_vectors;

    const int prime;

    map<tuple<vector<int>,vector<int>>, int> store;

    nmod_poly_t poly;
};

#endif
