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
#include <memory>
#include <vector>
#include <CL/cl.hpp>

#include <opencl_interface.hh>


using namespace std;


const std::string
kernel_evaluation_code =
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

const std::string
kernel_reduction_code =
  "void\n"
  "kernel\n"
  "reduce(\n"
  "  global int* as,\n"
  "  global int* bins,\n"
  "  const int len,\n"
  "  const int nmb_bins,\n"
  "  local int* partial_sum_a,\n"
  "  global int* sum_a\n"
  "  )\n"
  "{\n"
  "  int gsz = get_global_size(0);\n"
  "  int gix = get_global_id(0);\n"
  "  int lsz = get_local_size(0);\n"
  "  int lix = get_local_id(0);\n"
  "  int bsz = get_num_groups(0);\n"
  "  int nmb_groups = gsz / lsz;\n"
  "\n"
  "  int acc;\n"
  "  for (int bix=0; bix<bsz; ++bix) {\n"
  "    for (int ix=get_global_id(0); ix<len; ix+=gsz)\n"
  "      if (bins[ix] == bix) acc += as[ix];\n"
  "    partial_sum_a[lix + bix*lsz] = acc;\n"
  "  }\n"
  "\n"
  "  barrier(CLK_LOCAL_MEM_FENCE);\n"
  "  for (int offset = lsz/2; offset>0; offset/=2) {\n"
  "    if (lix < offset)\n"
  "      for (int bix=0; bix<bsz*lsz; bix+=lsz)\n"
  "        partial_sum_a[lix + bix] += partial_sum_a[lix+offset + bix];\n"
  "    barrier(CLK_LOCAL_MEM_FENCE);\n"
  "  }\n"
  "\n"
  "  if (lix == 0)\n"
  "    for (int bix=0; bix<bsz; ++bix)\n"
  "       sum_a[get_group_id(0) + bix*nmb_groups] = partial_sum_a[0 + bix*lsz];\n"
  "}\n"
  ;

OpenCLInterface::
OpenCLInterface()
{
  vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);
  if (all_platforms.size() == 0)
    throw "No platforms found.";
  this->platform = make_shared<cl::Platform>(all_platforms[0]);

  vector<cl::Device> all_devices;
  this->platform->getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
  if (all_devices.size() == 0)
    throw "No devices found.";
  this->device = make_shared<cl::Device>(all_devices[0]);

  this->context = make_shared<cl::Context>(*this->device);
  this->queue = make_shared<cl::CommandQueue>(*this->context, *this->device);

  cl::Program::Sources source_evaluation;
  source_evaluation.push_back({kernel_evaluation_code.c_str(), kernel_evaluation_code.length()});
  this->program_evaluation = make_shared<cl::Program>(*this->context, source_evaluation);
  if (this->program_evaluation->build({*this->device}) != CL_SUCCESS) {
    cerr << "Error building evaluation code:" << endl;
    cerr << this->program_evaluation->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*this->device);
    throw;
  }

  cl::Program::Sources source_reduction;
  source_reduction.push_back({kernel_reduction_code.c_str(), kernel_reduction_code.length()});
  this->program_reduction = make_shared<cl::Program>(*this->context, source_reduction);
  if (this->program_reduction->build({*this->device}) != CL_SUCCESS) {
    cerr << "Error building reduction code:" << endl;
    cerr << this->program_reduction->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*this->device);
    throw;
  }
}

