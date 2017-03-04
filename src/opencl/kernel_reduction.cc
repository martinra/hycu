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
  this->vector_nmbs.resize(prime_power_pred);
}

int
OpenCLKernelReduction::
_reduce(
    shared_ptr<cl::Buffer> buffer_nmbs
    )
{
  cl_int status;

  status = this->opencl->queue->enqueueReadBuffer( *buffer_nmbs, CL_TRUE,
               0, sizeof(int) * prime_power_pred, this->vector_nmbs.data());
  if ( status != CL_SUCCESS ) {
    cerr << "OpenCLKernelReduction::_reduce: could not read buffer_nmbs" << endl;
    throw;
  }

  return accumulate(this->vector_nmbs.cbegin(), this->vector_nmbs.cend(), 0);
}

tuple<int,int>
OpenCLKernelReduction::
reduce()
{
  return make_tuple(
    this->_reduce(this->buffer_nmbs_unramified),
    this->_reduce(this->buffer_nmbs_ramified)
    );
}
