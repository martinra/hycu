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

#include <cmath>

#ifdef TIMING
#include <chrono>
#endif

#include "opencl/interface.hh"
#include "reduction_table.hh"
#include "single_curve_fp.hh"


using namespace std;


shared_ptr<Curve>
single_curve_fp(
    unsigned int prime,
    vector<unsigned int> poly_coeffs,
    SingleCurveCountImplementation implementation
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

  shared_ptr<OpenCLInterface> opencl;
  if (  implementation == SingleCurveCountImplementationCPU
     || implementation == SingleCurveCountImplementationNaiveNMod
     || implementation == SingleCurveCountImplementationNaiveZech )
    opencl = shared_ptr<OpenCLInterface>();
#ifdef WITH_OPENCL
  else if ( implementation == SingleCurveCountImplementationOpenCL )
    opencl = make_shared<OpenCLInterface>();
#endif
  else {
    cerr << "curve count implementation not implemented" << endl;
    throw;
  }
  auto curve = make_shared<Curve>(enumeration_table, poly_coeff_exponents);

#ifdef TIMING
  chrono::steady_clock::time_point start;
#endif
  if (  implementation == SingleCurveCountImplementationNaiveNMod
     || implementation == SingleCurveCountImplementationNaiveZech ) {
    for ( size_t fx=curve->genus(); fx>0; --fx ) {
#ifdef TIMING
      start = chrono::steady_clock::now();
#endif
      if ( implementation == SingleCurveCountImplementationNaiveNMod )
        curve->count_naive_nmod(fx);
      if ( implementation == SingleCurveCountImplementationNaiveZech )
        curve->count_naive_zech(fx);
#ifdef TIMING
      cerr << "  TIMING: naive counting "
           << curve->prime_power() << "^" << fx << endl
           << "    "
           << chrono::duration<double, milli>(chrono::steady_clock::now() - start).count()
           << " ms" << endl;
#endif
    }
  }
  else {
    for ( size_t fx=curve->genus(); fx>curve->genus()/2; --fx ) {
#ifdef TIMING
      start = chrono::steady_clock::now();
#endif
      ReductionTable reduction_table(prime, fx, opencl);
#ifdef TIMING
      cerr << "  TIMING: reduction table "
           << curve->prime_power() << "^" << fx << endl
           << "    "
           << chrono::duration<double, milli>(chrono::steady_clock::now() - start).count()
           << " ms" << endl;
      start = chrono::steady_clock::now();
#endif
      curve->count(reduction_table);
#ifdef TIMING
      cerr << "  TIMING: counting total "
           << curve->prime_power() << "^" << fx << endl
           << "    "
           << chrono::duration<double, milli>(chrono::steady_clock::now() - start).count()
           << " ms" << endl;
#endif
    }
  }

  return curve;
}
