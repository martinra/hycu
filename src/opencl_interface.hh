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


#ifndef _H_OPENCL_INTERFACE
#define _H_OPENCL_INTERFACE

#include <memory>
#include <CL/cl.hpp>


using std::shared_ptr;


class OpenCLInterface
{
  public:
    OpenCLInterface() : OpenCLInterface ( OpenCLInterface::devices().front() );
    OpenCLInterface(cl::Device device);

    static vector<cl::Device> devices();

    friend class Curve;
    friend class ReductionTable;

  protected:

    // shared_ptr<cl::Platform> platform;
    shared_ptr<cl::Device> device;
    shared_ptr<cl::Context> context;
    shared_ptr<cl::CommandQueue> queue;

    shared_ptr<cl::Program> program_evaluation;
    shared_ptr<cl::Program> program_reduction;
};

#endif
