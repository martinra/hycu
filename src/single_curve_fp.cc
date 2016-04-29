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


#include <map>

#include "opencl_interface.hh"
#include "reduction_table.hh"
#include "single_curve_fp.hh"


using namespace std;


shared_ptr<Curve>
single_curve_fp(
    unsigned int prime,
    vector<unsigned int> poly_coeffs,
    bool use_opencl
    )
{
  auto enumeration_table = make_shared<FqElementTable>(prime, 1);

  unsigned int gen = enumeration_table->at_nmod(1);
  unsigned int a = 1;
  map<unsigned int, unsigned int> fp_conversion_map;
  fp_conversion_map[0] = prime-1;
  for (unsigned int ex=0; ex<prime-1; ++ex) {
    fp_conversion_map[a] = ex;
    a = (a*gen) % prime;
  }

  vector<int> poly_coeff_exponents;
  for ( auto c : poly_coeffs )
    poly_coeff_exponents.push_back(fp_conversion_map[c]);


  auto opencl = use_opencl ?
                  make_shared<OpenCLInterface>() : shared_ptr<OpenCLInterface>();
  auto curve = make_shared<Curve>(enumeration_table, poly_coeff_exponents);

  for ( size_t fx=curve->genus(); fx>curve->genus()/2; --fx ) {
    ReductionTable reduction_table(prime, fx, opencl);
    curve->count(reduction_table);
  }

  return curve;
}
