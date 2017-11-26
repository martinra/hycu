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

AS_STRING(

void
kernel
reduce(
  global const int * restrict as,
  const int len,
  local int * restrict partial_sum_a,
  global int * restrict sum_a
  )
{
  int gsz = get_global_size(0);
  int gix = get_global_id(0);
  int lsz = get_local_size(0);
  int lix = get_local_id(0);
  int nmb_groups = get_num_groups(0);

  int acc = 0;
  for (int ix=get_global_id(0); ix<len; ix+=gsz)
    acc += as[ix];
  partial_sum_a[lix] = acc;

  barrier(CLK_LOCAL_MEM_FENCE);
  for (int offset = lsz/2; offset>=SIZE_PARTIAL_REDUCTION; offset/=2) {
    if (lix < offset)
      partial_sum_a[lix] += partial_sum_a[lix+offset];
    barrier(CLK_LOCAL_MEM_FENCE);
  }

  if (lix < SIZE_PARTIAL_REDUCTION)
    sum_a[SIZE_PARTIAL_REDUCTION*get_group_id(0)+lix] = partial_sum_a[lix];
}

);
