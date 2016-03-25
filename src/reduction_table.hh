#ifndef _H_REDUCTION_TABLE
#define _H_REDUCTION_TABLE

#include <memory>
#include <CL/cl.hpp>
#include <opencl_interface.hh>


using namespace std;


class ReductionTable
{
  public:
    // the field size is prime_pow, which is a power of prime
    // field size is referred to as q, field prime is referred to as p
    // we write q = p^r
    // a fixed generator for F_q is referred to by a
    // in the current implemnentation it is given by the Conway polynomial
    // ReductionTable(int prime, int prime_exponent);
    ReductionTable(int prime, int prime_exponent, OpenCLInterface & opencl);

    // init_opencl_buffers(OpenCLInterface & opencl);
    
    // int inline zero_index() const { return this->prime_power - 1; };
    // vector<int> power_cosets(int n);

    friend class Curve;

  protected:
    const int prime;
    const int prime_exponent;
    const int prime_power;
    const int prime_power_pred;

    OpenCLInterface & opencl;

    // the reduction table is the reduction table modulo q-1 for integers less than max(r,2)*(q-1)
    shared_ptr<vector<int>> exponent_reduction_table;
    // given a^i, tabulate the mod q-1 reduced j with a^j = 1 + a^i,
    // if there is any, and q-1 if there is non
    shared_ptr<vector<int>> incrementation_table;
    // given an exponent determine the minimal prime exponent for which it occurs as an element
    shared_ptr<vector<int>> minimal_field_table;

    // opencl buffers that store the corresponding table
    shared_ptr<cl::Buffer> buffer_exponent_reduction_table;
    shared_ptr<cl::Buffer> buffer_incrementation_table;
    shared_ptr<cl::Buffer> buffer_minimal_field_table;

  private:
    shared_ptr<vector<int>> compute_exponent_reduction_table(int prime_power);
    shared_ptr<vector<int>>
        compute_incrementation_table(int prime, int prime_exponent, int prime_power);
    shared_ptr<vector<int>>
        compute_minimal_field_table(int prime, int prime_exponent, int prime_power);
};

#endif


