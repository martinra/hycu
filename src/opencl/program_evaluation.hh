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


#ifndef _H_OPENCL_PROGRAM_EVALUATION
#define _H_OPENCL_PROGRAM_EVALUATION

#include <memory>
#include <string>
#include <sstream>
#include <CL/cl.hpp>

#include "opencl/program.hh"


using std::endl;
using std::shared_ptr;
using std::string;
using std::stringstream;


class OpenCLInterface;


class OpenCLProgramEvaluation :
  public OpenCLProgram
{
  public:
    OpenCLProgramEvaluation(const OpenCLInterface & opencl, unsigned int degree) :
      degree ( degree )
    {
      this->init_program_cl(opencl);
    };

    friend class OpenCLKernelEvaluation;

  protected:
    shared_ptr<cl::Program> cl_program;

    const string code() const final { return this->_code; };
    const string function_name() const final { return this->_function_name; };

    const string build_options() const final
    {
      stringstream options;
      options << "-DPOLY_SIZE=" << (this->degree+1);
      return options.str();
    };

  private:
    const unsigned int degree;
    static const string _code;
    static const string _function_name;
};

#endif
