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


#include <chrono>

#include "opencl/interface.hh"
#include "reduction_table.hh"
#include "single_curve_fp.hh"


using namespace std;


shared_ptr<Curve>
single_curve_fp(
    unsigned int prime,
    vector<unsigned int> poly_coeffs,
    bool use_opencl,
    bool time
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

  // todo: use C++ interface for time
  chrono::duration<double, milli> reduction_table_duration, curve_count_duration;
  chrono::steady_clock::time_point start;
  for ( size_t fx=curve->genus(); fx>curve->genus()/2; --fx ) {
    if ( time )
      start = chrono::steady_clock::now();
    ReductionTable reduction_table(prime, fx, opencl);
    if ( time ) {
      reduction_table_duration += chrono::steady_clock::now() - start;
        // chrono::duration_cast<double, chrono::milliseconds>(chrono::steady_clock::now() - start);
      start = chrono::steady_clock::now();
    }
    curve->count(reduction_table);
    if ( time )
      curve_count_duration += chrono::steady_clock::now() - start;
//        chrono::duration_cast<double, chrono::milliseconds>(chrono::steady_clock::now() - start);
  }
  if ( time ) {
    cout << "Accumulated computation time for reduction tables: "
         << reduction_table_duration.count() << " ms" << endl;
    cout << "Accumulated computation time for curve couting: "
         << curve_count_duration.count() << " ms" << endl;
  }

  return curve;
}
