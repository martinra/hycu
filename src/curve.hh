#ifndef _H_CURVE
#define _H_CURVE

#include <memory>
#include <vector>
#include <tuple>
#include <CL/cl.hpp>

#include <reduction_table.hh>
#include <opencl_interface.hh>


using namespace std;


class Curve
{
  public:
    Curve(int prime, const vector<int>& poly_coeffs);

    int inline degree() const { return this->poly_coeffs.size() + 1; };
    int genus() const;

    bool has_squarefree_rhs();

    vector<int> poly_coefficients_as_generator_exponents(const ReductionTable & table);

    tuple<int,int> count(const ReductionTable & table, const OpenCLInterface&);

    vector<tuple<int,int>> isogeny_nmb_points(const vector<ReductionTable>& tables, const OpenCLInterface&);

    friend ostream& operator<<(ostream &stream, const Curve & curve);

  private:
    const int prime;
    vector<int> poly_coeffs;
};

// todo: rename to CurveBlockCounter
// todo: unify with CurveEnumerator
class CurveCounter
{
  public:
    CurveCounter(int prime, vector<tuple<int,int>> coeff_bounds);

    int inline genus() const { return genus_; };
    int inline degree() const { return coeff_bounds.size()-1; };

    void count(function<void(vector<int>&, vector<tuple<int,int>>&)>,
               const vector<ReductionTable> &, const OpenCLInterface &);

  private:
    const int prime;
    int genus_;

    vector<tuple<int,int>> coeff_bounds;

    void count_recursive(function<void(vector<int> &, vector<tuple<int,int>> &)>,
                         const vector<ReductionTable> &, const OpenCLInterface &, vector<int> & poly_coeffs);
};

#endif
