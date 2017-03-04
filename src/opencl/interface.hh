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

#ifndef WITH_OPENCL

class OpenCLInterface {};

#else

#include <map>
#include <memory>
#include <CL/cl.hpp>

#include "opencl/program_evaluation.hh"


using std::map;
using std::make_shared;
using std::shared_ptr;
using std::vector;


class OpenCLInterface
{
  public:
    OpenCLInterface() : OpenCLInterface ( OpenCLInterface::devices().front() ) {};
    OpenCLInterface(cl::Device device);

    static vector<cl::Device> devices();

    inline shared_ptr<OpenCLProgramEvaluation> program_evaluation(unsigned int degree)
    {
      const auto & program_it = this->_program_evaluation.find(degree);
      if ( program_it == this->_program_evaluation.end() ) {
        this->_program_evaluation[degree] = make_shared<OpenCLProgramEvaluation>(*this, degree);
        return this->_program_evaluation[degree];
      }
      else
        return program_it->second;
    };

    friend class Curve;
    friend class ReductionTable;
    friend class OpenCLProgram;
    friend class OpenCLBufferEvaluation;
    friend class OpenCLKernelEvaluation;
    friend class OpenCLKernelReduction;

  protected:

    shared_ptr<cl::Device> device;
    shared_ptr<cl::Context> context;
    shared_ptr<cl::CommandQueue> queue;

    map<unsigned int, shared_ptr<OpenCLProgramEvaluation>> _program_evaluation;
};

#endif // WITH_OPENCL

#endif
