#ifndef _H_BLOCK_ENUMERATOR
#define _H_BLOCK_ENUMERATOR

#include <vector>
#include <tuple>


using namespace std;


class CurveEnumerator
{
  public:
    CurveEnumerator(int prime, int genus, int package_size);

    int inline genus() const { return genus_; };
    int inline degree() const { return 2*genus_ + 2; };

    int inline is_end() const { return is_end_; };

    CurveEnumerator & step();

    vector<tuple<int,int>> as_bounds();

  private:
    const int prime;
    const int genus_;
    const int package_size;

    bool is_end_;
    size_t dx;
    vector<int> poly_coeffs;

    int fp_non_square();
};

#endif

