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


using namespace std;


OpenCLKernelEvaluation::
OpenCLKernelEvaluation(
    const ReductionTable & table,
    unsigned int degree
    ) :
  prime_power_pred ( table.prime_power_pred ),
  degree ( degree ),
  opencl ( table.opencl )
{
  const auto & function_name = this->opencl->program_evaluation(degree)->function_name();
  this->kernel_cl =
    make_shared<cl::Kernel>( *this->opencl->program_evaluation(degree)->program_cl,
                             function_name.c_str() );



  this->buffer_poly_coeff_exponents = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_ONLY, sizeof(int) * (degree+1) );


  cl_int status;

  status = this->kernel_cl->setArg(1, sizeof(int), &this->prime_power_pred);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set prime_power_pred" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(2, *table.buffer_evaluation()->buffer_exponent_reduction_table);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set exponent_reduction_table" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(3, *table.buffer_evaluation()->buffer_incrementation_table);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set incremetion_table" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(4, *table.buffer_evaluation()->buffer_minimal_field_table);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set minimal_field_table" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(5, *table.buffer_evaluation()->buffer_nmbs_unramified);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set nmbs_unramified" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(6, *table.buffer_evaluation()->buffer_nmbs_ramified);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set nmbs_ramified" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(7, *table.buffer_evaluation()->buffer_minimal_fields);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set minimal_fields" << endl;
    throw;
  }
}

void
OpenCLKernelEvaluation::
enqueue(
    vector<int> poly_coeff_exponents
    )
{
  cl_int status;

  if ( poly_coeff_exponents.size() != this->degree + 1 ) {
    cerr << "OpenCLKernelEvaluation::enqueue: size of coefficient vector must correspond to degree" << endl;
    throw;
  }

  status = this->opencl->queue->enqueueWriteBuffer(*this->buffer_poly_coeff_exponents, CL_FALSE, 0,
                                    sizeof(int) * (degree+1), poly_coeff_exponents.data());
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not write poly_coeff_exponents" << endl;
    throw;
  }


  status = this->kernel_cl->setArg(0, *this->buffer_poly_coeff_exponents);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not set poly_coeff_exponents" << endl;
    throw;
  }


  status = opencl->queue->enqueueNDRangeKernel( *this->kernel_cl,
               cl::NullRange, cl::NDRange(prime_power_pred), cl::NullRange );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not enqueue kernel" << endl;
    throw;
  }
}
