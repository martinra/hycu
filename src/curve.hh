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
    Curve(shared_ptr<FqElementTable> table, const vector<int> poly_coeff_exponents);

    int inline degree() const { return this->poly_coeff_exponents.size() + 1; };
    int genus() const;

    bool has_squarefree_rhs();
    nmod_poly_t rhs_flint_polynomial();
    fq_nmod_poly_t rhs_flint_polynomial();

    vector<int> convert_poly_coeff_exponents(const ReductionTable & table);

    const vector<tuple<int,int>> & count(const ReductionTable & table);

    vector<int> hasse_weil_offsets();
    vector<int> ramification_type();
    // todo: how does one compute the number of automorphisms?
    unsigned int nmb_automorphisms();

    friend ostream& operator<<(ostream &stream, const Curve & curve);

  private:
    const shared_ptr<FqElementTable> table;
    vector<int> poly_coeff_exponents;

    map<unsigned int, tuple<int,int>> nmb_points;
};

#endif
