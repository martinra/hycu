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

#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <CL/cl.hpp>

#include "opencl/interface.hh"


using namespace std;


const std::string
kernel_reduction_code =
  "void\n"
  "kernel\n"
  "reduce(\n"
  "  global const int* as,\n"
  "  global const int* bins,\n"
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
  "  int nmb_groups = get_num_groups(0);\n"
  "\n"
  "  int acc = 0;\n"
  "  for (int bix=0; bix<nmb_bins; ++bix) {\n"
  "    for (int ix=get_global_id(0); ix<len; ix+=gsz)\n"
  "      if (bins[ix] == bix) acc += as[ix];\n"
  "    partial_sum_a[lix + bix*lsz] = acc;\n"
  "  }\n"
  "\n"
  "  barrier(CLK_LOCAL_MEM_FENCE);\n"
  "  for (int offset = lsz/2; offset>0; offset/=2) {\n"
  "    if (lix < offset)\n"
  "      for (int bix=0; bix<nmb_bins*lsz; bix+=lsz)\n"
  "        partial_sum_a[lix + bix] += partial_sum_a[lix+offset + bix];\n"
  "    barrier(CLK_LOCAL_MEM_FENCE);\n"
  "  }\n"
  "\n"
  "  if (lix == 0)\n"
  "    for (int bix=0; bix<nmb_bins; ++bix)\n"
  "       sum_a[get_group_id(0) + bix*nmb_groups] = partial_sum_a[0 + bix*lsz];\n"
  "}\n"
  ;

OpenCLInterface::
OpenCLInterface(
    cl::Device device
    )
{
  this->device = make_shared<cl::Device>(device);
  this->context = make_shared<cl::Context>(*this->device);
  this->queue = make_shared<cl::CommandQueue>(*this->context, *this->device);

  this->program_evaluation = make_shared<OpenCLProgramEvaluation>(*this);

  cl::Program::Sources source_reduction;
  source_reduction.push_back({kernel_reduction_code.c_str(), kernel_reduction_code.length()});
  this->program_reduction = make_shared<cl::Program>(*this->context, source_reduction);
  if (this->program_reduction->build({*this->device}) != CL_SUCCESS) {
    cerr << "Error building reduction code:" << endl;
    cerr << this->program_reduction->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*this->device);
    throw;
  }
}

vector<cl::Device>
OpenCLInterface::
devices()
{
  vector<cl::Platform> platforms_all_vendors;
  cl::Platform::get(&platforms_all_vendors);

  vector<cl::Platform> platforms;
  vector<string> vendors;
  string vendor;
  for ( const auto & platform : platforms_all_vendors ) {
    platform.getInfo(CL_PLATFORM_VENDOR, &vendor);

    if ( find(vendors.begin(), vendors.end(), vendor) == vendors.end() ) {
      vendors.push_back(vendor);
      platforms.push_back(platform);
    }
  }

  vector<cl::Device> devices, platform_devices;
  for ( const auto & platform : platforms ) {
    platform_devices.clear();
    platform.getDevices(CL_DEVICE_TYPE_GPU, &platform_devices);
    devices.insert(devices.end(), platform_devices.begin(), platform_devices.end());

    platform_devices.clear();
    platform.getDevices(CL_DEVICE_TYPE_ACCELERATOR, &platform_devices);
    devices.insert(devices.end(), platform_devices.begin(), platform_devices.end());
  }

  return devices;
}
