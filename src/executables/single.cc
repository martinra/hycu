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


#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include "flint/ulong_extras.h"

#include "curve.hh"
#include "single_curve_fp.hh"


namespace popt = boost::program_options;
using namespace std;
using popt::value;


int
main(
    int argc,
    char** argv
    )
{
  popt::options_description hidden_options, visible_options("Available options"), all_options;
  popt::positional_options_description positional_options;

  hidden_options.add_options()
    ( "coefficients", value<vector<unsigned int>>()->composing(),
      "coefficients of the polynomial" )
    ( "field_size", value<unsigned int>(),
      "size of the ground field" );

  positional_options.add("field_size", 1)
                    .add("coefficients", -1);

  visible_options.add_options()
    ( "help,h", "show help message" )
    ( "opencl", "use OpenCL" );

  all_options.add(hidden_options)
             .add(visible_options);

  popt::variables_map options_map;
  popt::store( popt::command_line_parser(argc, argv)
                 .options(all_options)
                 .positional(positional_options)
                 .run(),
               options_map );
  popt::notify(options_map);


  if ( options_map.count("help") ) {
    cerr << visible_options;
    exit(0);
  }


  {
    bool invalid_options = false;
    if ( !options_map.count("field_size") ) {
      invalid_options = true;
      cerr << "no field size was specified";
    }
    else if ( !options_map.count("coefficients") ) {
      invalid_options = true;
      cerr << "no coefficient was specified";
    }
    else if ( options_map["coefficients"].as<vector<unsigned int>>().size() < 2 ) {
      invalid_options = true;
      cerr << "less than two coefficient were specified";
    }
    if ( invalid_options ) {
      cerr << ": call with at least 3 arguments" << endl;
      exit(1);
    }
  }

  if ( !n_is_prime(options_map["field_size"].as<unsigned int>()) ) {
    cerr << "field size is not prime";
    exit(1);
  }


  auto curve = single_curve_fp( options_map["field_size"].as<unsigned int>(),
                                options_map["coefficients"].as<vector<unsigned int>>(),
                                (bool)options_map.count("opencl") );
    
  cout << *curve << endl;

  cout << "coefficient exponents: ";
  auto coeff_exponents = curve->convert_poly_coeff_exponents(
      ReductionTable(curve->prime(), 1, make_shared<OpenCLInterface>()) );
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

