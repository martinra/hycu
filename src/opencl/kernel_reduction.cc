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

#include <iostream>
#include <numeric>

#include "reduction_table.hh"
#include "opencl/kernel_evaluation.hh"
#include "opencl/kernel_reduction.hh"


using namespace std;

OpenCLKernelReduction::
OpenCLKernelReduction(
    const ReductionTable & table
    ) :
  prime_exponent ( table.prime_exponent ),
  prime_power_pred ( table.prime_power_pred ),
  opencl ( table.opencl ),
  buffer_nmbs_unramified ( table.buffer_evaluation()->buffer_nmbs_unramified ),
  buffer_nmbs_ramified ( table.buffer_evaluation()->buffer_nmbs_ramified )
{
#if SIZE_PARTIAL_REDUCTION == 0
  this->sums.resize(prime_power_pred);

#else // SIZE_PARTIAL_REDUCTION != 0
  this->sums.resize(nmb_groups_reduction*SIZE_PARTIAL_REDUCTION);

  const auto & function_name = this->opencl->program_reduction()->function_name();
  this->kernel_cl = make_shared<cl::Kernel>(*this->opencl->program_reduction()->program_cl,
                                            function_name.c_str());


  this->buffer_sums = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_WRITE_ONLY, sizeof(int) * this->sums.size() );

  cl_int status;

  status = this->kernel_cl->setArg(1, sizeof(int), &this->prime_power_pred);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set prime_power_pred" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(2, sizeof(int) * this->local_size_reduction, nullptr);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set scratch" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(3, *this->buffer_sums);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set sums" << endl;
    throw;
  }
#endif
}

int
OpenCLKernelReduction::
_reduce(
    shared_ptr<cl::Buffer> buffer_nmbs
    )
{
  cl_int status;

#if SIZE_PARTIAL_REDUCTION == 0
  status = this->opencl->queue->enqueueReadBuffer( *buffer_nmbs, CL_TRUE,
               0, sizeof(int) * this->sums.size(), this->sums.data());
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not read buffer nmbs" << endl;
    throw;
  }

#else // SIZE_PARTIAL_REDUCTION != 0
  status = this->kernel_cl->setArg(0, *buffer_nmbs);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not set buffer_nmbs" << endl;
    throw;
  }


  status = this->opencl->queue->enqueueNDRangeKernel( *this->kernel_cl,
               cl::NullRange, cl::NDRange(this->global_size_reduction), cl::NDRange(this->local_size_reduction) );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not enqueue kernel" << endl;
    throw;
  }
  status = this->opencl->queue->finish();
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not finish queue:" << endl;
    if ( status == CL_OUT_OF_HOST_MEMORY )
      cerr << "  out of host memory" << endl;
    else if ( status == CL_INVALID_COMMAND_QUEUE )
      cerr << "  invalid command queue" << endl;
    throw;
  }

  status = this->opencl->queue->enqueueReadBuffer( *this->buffer_sums, CL_TRUE,
               0, sizeof(int) * this->sums.size(), this->sums.data());
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not read sums" << endl;
    throw;
  }
#endif

  return accumulate(this->sums.cbegin(), this->sums.cend(), 0);
}

void
OpenCLKernelReduction::
reduce(
    map<unsigned int, tuple<int,int>> & nmb_points
    )
{
  get<0>(nmb_points[this->prime_exponent]) +=
    this->_reduce(this->buffer_nmbs_unramified);

  get<1>(nmb_points[this->prime_exponent]) +=
    this->_reduce(this->buffer_nmbs_ramified);
}
