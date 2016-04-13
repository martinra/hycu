#ifndef _H_CURVE
#define _H_CURVE

#include <map>
#include <memory>
#include <vector>
#include <tuple>
#include <CL/cl.hpp>
#include <flint/fq_nmod_poly.h>
#include <flint/nmod_poly.h>

#include <fq_element_table.hh>
#include <reduction_table.hh>
#include <opencl_interface.hh>


using namespace std;


class Curve
{
  public:
    Curve(shared_ptr<FqElementTable> table, const vector<int> poly_coeff_exponents);

    unsigned int inline prime() const { return this->table->prime; };
    unsigned int inline prime_exponent() const { return this->table->prime_exponent; };
    unsigned int inline prime_power() const { return this->table->prime_power; };

    int inline degree() const { return this->poly_coeff_exponents.size() - 1; };
    int genus() const;

    bool has_squarefree_rhs();
    nmod_poly_struct rhs_nmod_polynomial() const;
    fq_nmod_poly_struct rhs_polynomial() const;

    vector<int> convert_poly_coeff_exponents(const ReductionTable & table);

    void count(const ReductionTable & table);
    bool has_counted(size_t fx) const { return (this->nmb_points.find(fx) != this->nmb_points.end()); };

    const map<unsigned int, tuple<int,int>> & number_of_points() const { return this->nmb_points; };
    vector<tuple<int,int>> number_of_points(unsigned int max_prime_exponent) const;

    map<unsigned int, int> hasse_weil_offsets() const;
    vector<int> hasse_weil_offsets(unsigned int max_prime_exponent) const;

    vector<int> ramification_type() const;
    // todo: how does one compute the number of automorphisms?
    unsigned int nmb_automorphisms();

    friend ostream& operator<<(ostream &stream, const Curve & curve);

    friend class IsogenyCountStore;
    friend class IsogenyRepresentativeStore;

  protected:
    const shared_ptr<FqElementTable> table;
    vector<int> poly_coeff_exponents;

    map<unsigned int, tuple<int,int>> nmb_points;
};

#endif
