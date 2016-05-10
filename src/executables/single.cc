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


#include <fstream>
#include <iostream>

#include "curve.hh"
#include "single_curve_fp.hh"


using namespace std;


int
main(
    int argc,
    char** argv
    )
{
  if (argc < 3) {
    cerr << "Arguments: prime, coefficients" << endl;
    exit(1);
  }

  int prime = atoi(argv[1]);
  if (prime < 0) throw;

  vector<unsigned int> poly_coeffs;
  for ( int ix=2; ix < argc; ++ix )
    poly_coeffs.push_back(atoi(argv[ix]));

  auto curve = single_curve_fp(prime, poly_coeffs, true);

    
  cout << *curve << endl;

  cout << "coefficient exponents: ";
  auto coeff_exponents = curve->convert_poly_coeff_exponents(
      ReductionTable(prime, 1, make_shared<OpenCLInterface>()) );
  for ( auto const& c : coeff_exponents )
    cout << c << " ";
  cout << endl;

  cout << "number of points: ";
  for ( auto const& pts : curve->number_of_points(curve->genus()) )
    cout << get<0>(pts) << " " << get<1>(pts) << ";  ";
  cout << endl;

  cout << "hasse-weil offsets: ";
  for ( auto const& o : curve->hasse_weil_offsets(curve->genus()) )
    cout << o << ";  ";
  cout << endl;

  cout << "ramification type: ";
  for ( auto const& r : curve->ramification_type() )
    cout << r << ",";
  cout << endl;


  return 0;
}

