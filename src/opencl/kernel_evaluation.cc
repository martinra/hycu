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
    const ReductionTable & table
    ) :
  prime_power_pred ( table.prime_power_pred ),
  opencl ( table.opencl )
{
  const auto & function_name = this->opencl->program_evaluation->function_name();
  this->kernel_cl = make_shared<cl::Kernel>(*this->opencl->program_evaluation->program_cl,
                                            function_name.c_str());

  this->buffer_exponent_reduction_table = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_ONLY, sizeof(int) * table.exponent_reduction_table->size() );
  this->buffer_incrementation_table = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_ONLY, sizeof(int) * table.incrementation_table->size() );
  this->buffer_minimal_field_table = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_ONLY, sizeof(int) * table.minimal_field_table->size() );

  this->buffer_nmbs_unramified = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_WRITE, sizeof(int) * table.prime_power_pred);
  this->buffer_nmbs_ramified = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_WRITE, sizeof(int) * table.prime_power_pred);
  this->buffer_minimal_fields = make_shared<cl::Buffer>(
      *this->opencl->context, CL_MEM_READ_WRITE, sizeof(int) * table.prime_power_pred);

  cl_int status;

  status = this->opencl->queue->enqueueWriteBuffer(*this->buffer_exponent_reduction_table, CL_TRUE, 0,
      sizeof(int) * table.exponent_reduction_table->size(),
      table.exponent_reduction_table->data() );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not write reduction_table" << endl;
    throw;
  }

  status = this->opencl->queue->enqueueWriteBuffer(*this->buffer_incrementation_table, CL_TRUE, 0,
      sizeof(int) * table.incrementation_table->size(),
      table.incrementation_table->data() );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not write incrementation_table" << endl;
    throw;
  }

  status = this->opencl->queue->enqueueWriteBuffer(*this->buffer_minimal_field_table, CL_TRUE, 0,
      sizeof(int) * table.minimal_field_table->size(),
      table.minimal_field_table->data() );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not write minimal_field_table" << endl;
    throw;
  }


  int prime_power_pred = table.prime_power_pred;
  status = this->kernel_cl->setArg(2, sizeof(int), &prime_power_pred);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set prime_power_pred" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(3, *this->buffer_exponent_reduction_table);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set exponent_reduction_table" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(4, *this->buffer_incrementation_table);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set incremetion_table" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(5, *this->buffer_minimal_field_table);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set minimal_field_table" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(6, *this->buffer_nmbs_unramified);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set nmbs_unramified" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(7, *this->buffer_nmbs_ramified);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set nmbs_ramified" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(8, *this->buffer_minimal_fields);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation: could not set minimal_fields" << endl;
    throw;
  }
};

void
OpenCLKernelEvaluation::
enqueue(
    vector<int> poly_coeff_exponents
    )
{
  cl_int status;

  int poly_size = (int)poly_coeff_exponents.size();

  cl::Buffer buffer_poly_coeff_exponents(
               *this->opencl->context, CL_MEM_READ_ONLY,
               sizeof(int) * poly_size);
  status = this->opencl->queue->enqueueWriteBuffer(buffer_poly_coeff_exponents, CL_TRUE, 0,
                                    sizeof(int) * poly_size, poly_coeff_exponents.data());
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not write poly_coeff_exponents" << endl;
    throw;
  }


  status = this->kernel_cl->setArg(0, buffer_poly_coeff_exponents);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not set poly_coeff_exponents" << endl;
    throw;
  }

  status = this->kernel_cl->setArg(1, sizeof(int), &poly_size);
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not set poly_size" << endl;
    throw;
  }


  status = opencl->queue->enqueueNDRangeKernel( *this->kernel_cl,
               cl::NullRange, cl::NDRange(prime_power_pred), cl::NullRange );
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelEvaluation::enqueue: could not enqueue kernel" << endl;
    throw;
  }
}
