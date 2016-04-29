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
#include <string>
#include <sstream>
#include <tuple>
#include <yaml-cpp/yaml.h>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>
#include <block_iterator.hh>
#include <store/legacy.hh>


using namespace std;


int
main(
    int argc,
    char** argv
    )
{
  if (argc != 5) {
    cerr << "Four arguments, prime, degree, input, and output file must be given" << endl;
    exit(1);
  }

  int prime = atoi(argv[1]);
  if (prime < 0) throw;
  auto fq_table = make_shared<FqElementTable>(prime, 1);

  int degree = atoi(argv[2]);
  if (degree < 0) throw;
  int genus = degree & 1 ? (degree - 1)/2 : (degree - 2)/2;

  auto input_name = string(argv[3]);
  vector<tuple<int,int>> coefficient_bounds;
  if (input_name.length() > 5 && input_name.substr(input_name.length()-5,5) == ".yaml") {
    YAML::Node input = YAML::LoadFile(input_name);
    for (size_t ix = 0; ix<=degree; ++ix) {
      int lbd, ubd;

      if (!input[to_string(ix)]) {
        lbd = 0;
        ubd = prime;
      } else {
        if (!input[to_string(ix)]["lower"])
          lbd = 0;
        else
          lbd = input[to_string(ix)]["lower"].as<int>();

        if (!input[to_string(ix)]["upper"])
          ubd = prime;
        else
          ubd = input[to_string(ix)]["upper"].as<int>();
      }
      coefficient_bounds.push_back(make_tuple(lbd,ubd));
    }
  }
  else {
    coefficient_bounds.resize(degree+1);
    cerr << "reading bounds" << endl;

    fstream input(input_name, ios_base::in);
    string line, nmb;

    while ( getline(input, line) ) {
      cerr << "reading line: " << line << endl;
      stringstream line_stream(line);

      getline(line_stream, nmb, ' ');
      int ix = atoi(nmb.c_str());

      if (ix > degree) {
        cerr << "indices in input file too large" << endl;
        throw;
      }

      getline(line_stream, nmb, ' ');
      int lbd = atoi(nmb.c_str());

      getline(line_stream, nmb, ' ');
      int ubd = atoi(nmb.c_str()) + 1;

      cerr << "evaluated to: " << ix << " " << lbd << " " << ubd << endl;
      if (lbd >= ubd) {
        cerr << "lower bound must be less than or equal to upper one " << ix <<  " " << lbd << " " << ubd << endl;
        throw;
      }

      coefficient_bounds[ix] = make_tuple(lbd, ubd);
    }

  for ( auto & bd : coefficient_bounds )
    if ( get<0>(bd) == 0 && get<1>(bd) == 0 )
      bd = make_tuple(fq_table->zero_index(), fq_table->zero_index() + 1);
  }

  cerr << "read data" << endl;
  for ( auto const& bd : coefficient_bounds )
    cerr << get<0>(bd) << " " << get<1>(bd) << " ; ";
  cerr << endl;

  fstream output(argv[4], ios_base::out);


  auto opencl = make_shared<OpenCLInterface>();
  StoreLegacy isogeny_count_store;

  vector<ReductionTable> reduction_tables;
  for ( size_t fx=genus; fx>genus/2; --fx )
    reduction_tables.emplace_back(prime, fx, opencl);


  for ( BlockIterator iter(coefficient_bounds); !iter.is_end(); iter.step() ) {
    Curve curve(fq_table, iter.as_position());
    if ( !curve.has_squarefree_rhs() ) continue;
    for ( auto & table : reduction_tables ) curve.count(table);
    isogeny_count_store.register_curve(curve);
  }

  isogeny_count_store.output_legacy(output);

  return 0;
}
