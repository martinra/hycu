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
#include "opencl/program.hh"


using namespace std;


void
OpenCLProgram::
init_program_cl(
    const OpenCLInterface & opencl
    )
{
  auto code = this->code();
  cl::Program::Sources source; source.push_back({code.c_str(), code.length()});

  this->program_cl = make_shared<cl::Program>(*opencl.context, source);
  if (this->program_cl->build({*opencl.device}) != CL_SUCCESS) {
    cerr << "Error building code for function \"" << this->function_name() << "\":" << endl;
    cerr << this->program_cl->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*opencl.device);
    throw;
  }
}

