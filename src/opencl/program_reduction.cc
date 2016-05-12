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
#include "opencl/program_reduction.hh"


using namespace std;


const string
OpenCLProgramReduction::
_function_name =
  "reduce";

const string
OpenCLProgramReduction::
_code =
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

