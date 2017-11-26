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
evaluate(
  global const int * restrict poly_coeffs_exponents,
  const int prime_power_pred, // this is prime_power - 1
  
  global const int * restrict exponent_reduction_table,
  global const int * restrict incrementation_table,
  
  global int * restrict nmbs_unramified,
  global int * restrict nmbs_ramified
  )
{
  // The variable x = a^i is represented by i < prime_power_pred.
  // The case x=0 will not occur, but any element 0 is represented by then number prime_power_pred.
  int x = get_global_id(0);
  
  int f = poly_coeffs_exponents[0];
  int xpw = 0;
  #pragma unroll
  for (int dx=1; dx < POLY_SIZE; ++dx) {
    xpw += x;
    xpw = exponent_reduction_table[xpw];
    if (poly_coeffs_exponents[dx] != prime_power_pred) { // i.e. coefficient is not zero
      if (f == prime_power_pred) // i.e. f = 0
        f = exponent_reduction_table[poly_coeffs_exponents[dx] + xpw];
      else {
        int tmp = exponent_reduction_table[poly_coeffs_exponents[dx] + xpw];
        
        int tmp2;
        if (tmp <= f) {
          // this can be removed by doubling the size of incrementation_table
          // and checking at prime_power_pred + tmp - f
          tmp2 = f;
          f = tmp;
          tmp = tmp2;
        }
        tmp2 = incrementation_table[tmp-f];
        if (tmp2 != prime_power_pred) {
          f = f + tmp2;
          f = exponent_reduction_table[f];
        } else
          f = prime_power_pred;
      }
    }
  }
  
  if (f == prime_power_pred) {
    nmbs_unramified[x] = 0;
    nmbs_ramified[x] = 1;
  } else if (f & 1) {
    nmbs_unramified[x] = 0;
    nmbs_ramified[x] = 0;
  } else {
    nmbs_unramified[x] = 2;
    nmbs_ramified[x] = 0;
  }
}

);
