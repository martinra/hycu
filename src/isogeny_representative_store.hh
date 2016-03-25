#ifndef _H_ISOGENY_REPRESENTATIVE_STORE
#define _H_ISOGENY_REPRESENTATIVE_STORE

#include <map>
#include <memory>
#include <tuple>
#include <vector>

using namespace std;


class IsogenyRepresentativeStore
{
  public:
    void register_curve(const Curve & curve);

    friend ostream & operator<<(ostream & stream, const IsogenyRepresentativeStore & store);

  private:
    map<tuple<vector<int>,vector<int>>, vector<int>> store;
};

#endif
