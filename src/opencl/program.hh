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


#ifndef _H_OPENCL_PROGRAM
#define _H_OPENCL_PROGRAM

#include <memory>
#include <string>
#include <CL/cl.hpp>


class OpenCLInterface;

using std::shared_ptr;
using std::string;


class OpenCLProgram
{
  protected:
    shared_ptr<cl::Program> program_cl;

    OpenCLProgram() {};
    void init_program_cl(const OpenCLInterface & opencl);

  private:
    virtual const string code() const = 0;
    virtual const string function_name() const = 0;
    virtual const string build_options() const
    {
      return string();
    };
};

#endif
