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


OpenCLInterface::
OpenCLInterface(
    cl::Device device
    )
{
  this->device = make_shared<cl::Device>(device);
  this->context = make_shared<cl::Context>(*this->device);
  this->queue = make_shared<cl::CommandQueue>(*this->context, *this->device);

  this->_program_reduction = make_shared<OpenCLProgramReduction>(*this);
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
