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


#ifndef _H_OPENCL_BUFFER_EVALUATION
#define _H_OPENCL_BUFFER_EVALUATION

#include <memory>
#include <CL/cl.hpp>


using std::shared_ptr;


class ReductionTable;

class OpenCLBufferEvaluation
{
  public:
    OpenCLBufferEvaluation(const ReductionTable & table);

    friend class OpenCLKernelEvaluation;
    friend class OpenCLKernelReduction;

  protected:
    shared_ptr<cl::Buffer> buffer_exponent_reduction_table;
    shared_ptr<cl::Buffer> buffer_incrementation_table;

    shared_ptr<cl::Buffer> buffer_nmbs_unramified;
    shared_ptr<cl::Buffer> buffer_nmbs_ramified;

  private:
    unsigned int prime_power_pred;

    shared_ptr<OpenCLInterface> opencl;
};

#endif
