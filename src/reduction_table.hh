/*============================================================================

    (C) 2016 Martin Westerholt-Raum

    This file is part of HyCu.

    HyCu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    HyCu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with HyCu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

===============================================================================*/


#ifndef _H_REDUCTION_TABLE
#define _H_REDUCTION_TABLE

#include <memory>
#include <vector>
#include <CL/cl.hpp>

#include "opencl/interface.hh"
#include "opencl/kernel_evaluation.hh"
#include "opencl/kernel_reduction.hh"


using std::shared_ptr;
using std::vector;


class ReductionTable
{
  public:
    // the field size is prime_power, which is a power of prime
    // field size is referred to as q, field prime is referred to as p
    // we write q = p^r
    // a fixed generator for F_q is referred to by a
    // in the current implemnentation it is given by the Conway polynomial
    ReductionTable(int prime, int prime_exponent)
      : ReductionTable(prime, prime_exponent, shared_ptr<OpenCLInterface>()) {};
    ReductionTable(int prime, int prime_exponent, shared_ptr<OpenCLInterface> && opencl);
    ReductionTable(int prime, int prime_exponent, const shared_ptr<OpenCLInterface> & opencl);

    void compute_tables();
    inline bool is_opencl_enabled() const { return (bool)opencl; };
    
    friend class Curve;
    friend OpenCLKernelEvaluation;
    friend OpenCLKernelReduction;

  protected:
    const int prime;
    const int prime_exponent;
    const int prime_power;
    const int prime_power_pred;

    shared_ptr<OpenCLInterface> opencl;
    shared_ptr<OpenCLKernelEvaluation> kernel_evaluation;
    shared_ptr<OpenCLKernelReduction> kernel_reduction;

    // the reduction table is the reduction table modulo q-1 for integers less than max(r,2)*(q-1)
    shared_ptr<vector<int>> exponent_reduction_table;
    // given a^i, tabulate the mod q-1 reduced j with a^j = 1 + a^i,
    // if there is any, and q-1 if there is non
    shared_ptr<vector<int>> incrementation_table;
    // given an exponent determine the minimal prime exponent for which it occurs as an element
    shared_ptr<vector<int>> minimal_field_table;

  private:
    shared_ptr<vector<int>> compute_exponent_reduction_table(int prime_power);
    shared_ptr<vector<int>>
        compute_incrementation_table(int prime, int prime_exponent, int prime_power);
    shared_ptr<vector<int>>
        compute_minimal_field_table(int prime, int prime_exponent, int prime_power);
};

#endif


