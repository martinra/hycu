#ifndef _H_REDUCTION_TABLE
#define _H_REDUCTION_TABLE

#include <memory>
#include <CL/cl.hpp>

using namespace std;


class Curve;

class ReductionTable
{
  public:
    // the field size is prime_pow, which is a power of prime
    // field size is referred to as q, field prime is referred to as p
    // we write q = p^r
    // a fixed generator for F_q is referred to by a
    // in the current implemnentation it is given by the Conway polynomial
    ReductionTable(int prime, int prime_exponent, OpenCLInterface & opencl);

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

#endif


