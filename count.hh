#ifndef _H_COUNT
#define _H_COUNT

#include <memory>
#include <CL/cl.hpp>


using namespace std;


class ReductionTableFq;
class Curve;

class OpenCLInterface
{
  public:
    OpenCLInterface();

    friend ReductionTableFq;
    friend Curve;

  protected:

    shared_ptr<cl::Platform> platform;
    shared_ptr<cl::Device> device;
    shared_ptr<cl::Context> context;
    shared_ptr<cl::CommandQueue> queue;

    shared_ptr<cl::Program> program_evaluation;
    shared_ptr<cl::Program> program_reduction;
};


class ReductionTableFq
{
  public:
    // the field size is prime_pow, which is a power of prime
    // field size is referred to as q, field prime is referred to as p
    // we write q = p^r
    // a fixed generator for F_q is referred to by a
    // in the current implemnentation it is given by the Conway polynomial
    ReductionTableFq(int prime, int prime_exponent, OpenCLInterface & opencl);

    friend Curve;

  protected:

    const int prime;
    const int prime_exponent;
    const int prime_power;

    // the reduction table is the reduction table modulo q-1 for integers less than max(r,2)*(q-1)
    shared_ptr<vector<int>> exponent_reduction_table;
    // given a^i, tabulate the mod q-1 reduced j with a^j = 1 + a^i,
    // if there is any, and q-1 if there is non
    shared_ptr<vector<int>> incrementation_table;

    // given an element in fp give the exponent of a that corresponds to it
    // 0 is mapped to q-1
    shared_ptr<vector<int>> fp_exponents;

    // opencl buffers that store the corresponding table
    shared_ptr<cl::Buffer> buffer_exponent_reduction_table;
    shared_ptr<cl::Buffer> buffer_incrementation_table;

  private:
    shared_ptr<vector<int>> compute_exponent_reduction_table(int prime_power);
    tuple<shared_ptr<vector<int>>, shared_ptr<vector<int>>>
        compute_incrementation_fp_exponents_tables(int prime, int prime_exponent, int prime_power);
};


class Curve
{
  public:
    Curve(int prime, const vector<int>& poly_coeffs);

    int inline degree() const { return this->poly_coeffs.size() + 1; };
    int genus() const;

    vector<int> poly_coefficients_as_powers(const ReductionTableFq & table);

    tuple<int,int> count(const ReductionTableFq & table, const OpenCLInterface&);

    vector<tuple<int,int>> isogeny_nmb_points(const vector<ReductionTableFq>& tables, const OpenCLInterface&);

    friend ostream& operator<<(ostream &stream, const Curve & curve);

  private:
    const int prime;
    vector<int> poly_coeffs;
};

class CurveCounter
{
  public:
    CurveCounter(int prime, int genus, vector<tuple<int,int>> coeff_bounds) :
      prime( prime ), genus_( genus ),  coeff_bounds( coeff_bounds ) {};

    int inline genus() const { return genus_; };
    int inline degree() const { return 2*genus_ + 2; };

    void count(function<void(vector<int>&, vector<tuple<int,int>>&)>,
               const vector<ReductionTableFq> &, const OpenCLInterface &);

  private:
    const int prime;
    const int genus_;

    vector<tuple<int,int>> coeff_bounds;

    void count_recursive(function<void(vector<int> &, vector<tuple<int,int>> &)>,
                         const vector<ReductionTableFq> &, const OpenCLInterface &, vector<int> & poly_coeffs);
};

#endif
