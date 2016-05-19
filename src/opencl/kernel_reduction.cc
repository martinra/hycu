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
  this->sums.resize(nmb_groups_reduction*prime_exponent);


  const auto & function_name = this->opencl->program_reduction()->function_name();
  this->kernel_cl = make_shared<cl::Kernel>(*this->opencl->program_reduction()->program_cl,
                                            function_name.c_str());


  this->buffer_sums = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_WRITE_ONLY, sizeof(int) * this->nmb_groups_reduction * this->prime_exponent );

  cl_int status;

  status = this->kernel_cl->setArg(1, *table.buffer_evaluation()->buffer_minimal_fields);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set minimal_fields" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(2, sizeof(int), &this->prime_power_pred);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set prime_power_pred" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(3, sizeof(int), &this->prime_exponent);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set prime_exponent" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(4, sizeof(int) * this->local_size_reduction * this->prime_exponent, nullptr);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set scratch" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(5, *this->buffer_sums);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction: could not set sums" << endl;
    throw;
  }
}

void
OpenCLKernelReduction::
_reduce(
    shared_ptr<cl::Buffer> buffer_nmbs
    )
{
  cl_int status;

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
               0, sizeof(int) * nmb_groups_reduction * prime_exponent, this->sums.data());
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not read sums unramified" << endl;
    throw;
  }
}

void
OpenCLKernelReduction::
reduce(
    map<unsigned int, tuple<int,int>> & nmb_points
    )
{
  this->_reduce(this->buffer_nmbs_unramified);

  for (size_t fx=1; fx<=prime_exponent; ++fx)
    if ( prime_exponent % fx == 0 )
      for (size_t ix=(fx-1)*nmb_groups_reduction; ix<fx*nmb_groups_reduction; ++ix)
        get<0>(nmb_points[fx]) += sums[ix];


  this->_reduce(this->buffer_nmbs_ramified);

  for (size_t fx=1; fx<=prime_exponent; ++fx)
    if ( prime_exponent % fx == 0 )
      for (size_t ix=(fx-1)*nmb_groups_reduction; ix<fx*nmb_groups_reduction; ++ix)
        get<1>(nmb_points[fx]) += sums[ix];
}
