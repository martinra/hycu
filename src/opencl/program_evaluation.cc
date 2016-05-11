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
#include "opencl/program_evaluation.hh"


using namespace std;


OpenCLProgramEvaluation::
OpenCLProgramEvaluation(
    const OpenCLInterface & opencl
    )
{
  cl::Program::Sources source;
  source.push_back({OpenCLProgramEvaluation::code.c_str(),
                    OpenCLProgramEvaluation::code.length()});

  this->cl_program = make_shared<cl::Program>(*opencl.context, source);
  if (this->cl_program->build({*opencl.device}) != CL_SUCCESS) {
    cerr << "Error building evaluation code:" << endl;
    cerr << this->cl_program->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*opencl.device);
    throw;
  }
}

const string
OpenCLProgramEvaluation::
function_name =
  "evaluate";

const string
OpenCLProgramEvaluation::
code =
  "void\n"
  "kernel\n"
  "evaluate(\n"
  "  global const int * poly_coeffs_exponents,\n"
  "  const int poly_size,\n"
  "  const int prime_power_pred, // this is prime_power - 1\n"
  "  \n"
  "  global const int * exponent_reduction_table,\n"
  "  global const int * incrementation_table,\n"
  "  global const int * minimal_field_table,\n"
  "  \n"
  "  global int * nmbs_unramified,\n"
  "  global int * nmbs_ramified,\n"
  "  global int * minimal_fields\n"
  "  )\n"
  "{\n"
  "  // The variable x = a^i is represented by i < prime_power_pred.\n"
  "  // The case x=0 will not occur, but any element 0 is represented by then number prime_power_pred.\n"
  "  int x = get_global_id(0);\n"
  "  \n"
  "  int f = poly_coeffs_exponents[0];\n"
  "  for (int dx=1, xpw=x; dx < poly_size; ++dx, xpw+=x) {\n"
  "    xpw = exponent_reduction_table[xpw];\n"
  "    if (poly_coeffs_exponents[dx] != prime_power_pred) { // i.e. coefficient is not zero\n"
  "      if (f == prime_power_pred) { // i.e. f = 0\n"
  "        f = poly_coeffs_exponents[dx] + xpw;\n"
  "        f = exponent_reduction_table[f];\n"
  "      } else {\n"
  "        int tmp = exponent_reduction_table[poly_coeffs_exponents[dx] + xpw];\n"
  "        \n"
  "        int tmp2;\n"
  "        if (tmp <= f) {\n"
  "          // this can be removed by doubling the size of incrementation_table\n"
  "          // and checking at prime_power_pred + tmp - f\n"
  "          tmp2 = f;\n"
  "          f = tmp;\n"
  "          tmp = tmp2;\n"
  "        }\n"
  "        tmp2 = incrementation_table[tmp-f];\n"
  "        if (tmp2 != prime_power_pred) {\n"
  "          f = f + tmp2;\n"
  "          f = exponent_reduction_table[f];\n"
  "        } else\n"
  "          f = prime_power_pred;\n"
  "      }\n"
  "    }\n"
  "  }\n"
  "  \n"
  "  int minimal_field_x = minimal_field_table[x];\n"
  "  int minimal_field_f;\n"
  "  \n"
  "  if (f == prime_power_pred) {\n"
  "    nmbs_unramified[x] = 0;\n"
  "    nmbs_ramified[x] = 1;\n"
  "    minimal_field_f = 0;\n"
  "  } else if (f & 1) {\n"
  "    nmbs_unramified[x] = 0;\n"
  "    nmbs_ramified[x] = 0;\n"
  "    minimal_field_f = 0;\n"
  "  } else {\n"
  "    nmbs_unramified[x] = 2;\n"
  "    nmbs_ramified[x] = 0;\n"
  "    minimal_field_f = minimal_field_table[f/2];\n"
  "  }\n"
  "\n"
  "  minimal_fields[x] = minimal_field_x > minimal_field_f ?\n"
  "                        minimal_field_x : minimal_field_f;\n"
  "}\n"
  ;
