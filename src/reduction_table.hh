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

#include "opencl/interface.hh"

#ifdef WITH_OPENCL
  #include "opencl/buffer_evaluation.hh"
  #include "opencl/kernel_evaluation.hh"
  #include "opencl/kernel_reduction.hh"
#endif


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
    ReductionTable(unsigned int prime, unsigned int prime_exponent)
      : ReductionTable(prime, prime_exponent, shared_ptr<OpenCLInterface>()) {};
    ReductionTable(unsigned int prime, unsigned int prime_exponent, shared_ptr<OpenCLInterface> && opencl);
    ReductionTable(unsigned int prime, unsigned int prime_exponent, const shared_ptr<OpenCLInterface> & opencl);

    void compute_tables();
    inline bool is_opencl_enabled() const { return (bool)opencl; };
    
    friend class Curve;
#ifdef WITH_OPENCL
    friend OpenCLBufferEvaluation;
    friend OpenCLKernelEvaluation;
    friend OpenCLKernelReduction;
#endif

  protected:
    const unsigned int prime;
    const unsigned int prime_exponent;
    const unsigned int prime_power;
    const unsigned int prime_power_pred;

    shared_ptr<OpenCLInterface> opencl;

    // the reduction table is the reduction table modulo q-1 for integers less than max(r,2)*(q-1)
    shared_ptr<vector<int32_t>> exponent_reduction_table;
    // given a^i, tabulate the mod q-1 reduced j with a^j = 1 + a^i,
    // if there is any, and q-1 if there is non
    shared_ptr<vector<int32_t>> incrementation_table;

#ifdef WITH_OPENCL
    inline shared_ptr<OpenCLBufferEvaluation> buffer_evaluation() const
    {
      return this->_buffer_evaluation;
    };

    inline shared_ptr<OpenCLKernelEvaluation> kernel_evaluation(unsigned int degree)
    {
      auto kernel_it = this->_kernel_evaluation.find(degree);
      if ( kernel_it == this->_kernel_evaluation.end() ) {
        this->_kernel_evaluation[degree] = make_shared<OpenCLKernelEvaluation>(*this, degree);
        return this->_kernel_evaluation[degree];
      }
      else
        return kernel_it->second;
    };

    inline shared_ptr<OpenCLKernelReduction> kernel_reduction() const
    {
      return this->_kernel_reduction;
    };
#endif

  private:
    shared_ptr<vector<int32_t>> compute_exponent_reduction_table(unsigned int prime_power);
    shared_ptr<vector<int32_t>>
        compute_incrementation_table(unsigned int prime, unsigned int prime_exponent, unsigned int prime_power);

#ifdef WITH_OPENCL
    shared_ptr<OpenCLBufferEvaluation> _buffer_evaluation;
    map<unsigned int, shared_ptr<OpenCLKernelEvaluation>> _kernel_evaluation;
    shared_ptr<OpenCLKernelReduction> _kernel_reduction;
#endif
};

#endif


