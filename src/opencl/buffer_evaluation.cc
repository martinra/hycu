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

#include "opencl/interface.hh"
#include "opencl/buffer_evaluation.hh"
#include "reduction_table.hh"


using namespace std;


OpenCLBufferEvaluation::
OpenCLBufferEvaluation(
    const ReductionTable & table
    ) :
  prime_power_pred ( table.prime_power_pred ),
  opencl ( table.opencl )
{
  this->buffer_exponent_reduction_table = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_ONLY, sizeof(int) * table.exponent_reduction_table->size() );
  this->buffer_incrementation_table = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_ONLY, sizeof(int) * table.incrementation_table->size() );

  this->buffer_nmbs_unramified = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_WRITE, sizeof(int) * table.prime_power_pred);
  this->buffer_nmbs_ramified = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_WRITE, sizeof(int) * table.prime_power_pred);


  cl_int status;
 
  status = this->opencl->queue->enqueueWriteBuffer(*this->buffer_exponent_reduction_table, CL_FALSE, 0,
      sizeof(int) * table.exponent_reduction_table->size(),
      table.exponent_reduction_table->data() );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not write reduction_table" << endl;
    throw;
  }

  status = this->opencl->queue->enqueueWriteBuffer(*this->buffer_incrementation_table, CL_FALSE, 0,
      sizeof(int) * table.incrementation_table->size(),
      table.incrementation_table->data() );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not write incrementation_table" << endl;
    throw;
  }
}
