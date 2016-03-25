#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <tuple>
#include <yaml.h>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>
#include <block_enumerator.hh>
#include <isogeny_type_store.hh>

using namespace std;


vector<tuple<int,int>> parse_input(int prime, int genus, istream& stream);
vector<tuple<int,int>> parse_legacy_input(int prime, int genus, istream& stream);


int
main(
    int argc,
    char** argv
    )
{
  if (argc != 5) {
    cerr << "Four arguments, prime, genus, input, and output file must be given" << endl;
    exit(1);
  }

  int prime = atoi(argv[1]);
  if (prime < 0) throw;

  int degree = atoi(argv[2]);
  if (degree < 0) throw;
  int genus = degree & 1 ? (degree - 1)/2 : (degree - 2)/2;

  auto input_name = string(argv[3]);
  vector<tuple<int,int>> coefficient_bounds;
  if (input_name.length() < 5 || input_name.substr(input_name.length()-5,5) == ".yaml") {
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
  } else {
    fstream input(input_name, ios_base::in);
    coefficient_bounds = parse_legacy_input(prime, degree, input);
  }


  fstream output(argv[4], ios_base::out);

  auto enumeration_table = make_shared<FqElementTable>(prime, 1);
  OpenCLInterface opencl();
  ReductionTable reduction_table(prime, genus, opencl);
  IsogenyTypeStore isogeny_type_store(prime);
  // note: we cannot convert between exponent blocks and additive blocks;
  // here we use exponent blocks
  enumerator = BlockIterator( coefficient_bounds );

  for (; !enumerator.is_end(); enumerator.step() ) {
    auto poly_coeff_exponents = enumerator.as_position()
    Curve curve(enumeration_table, poly_coeff_exponents);
    if ( !curve.has_squarefree_rhs() ) continue;

    curve.count(reduction_table);
    isogeny_type_store.register_curve(curve);
  }

  isogeny_type_store.output_legacy(output);

  return 0;
}

vector<tuple<int,int>>
parse_legacy_input(
    int prime,
    int degree,
    istream& stream
    )
{
  vector<tuple<int,int>> coeff_bounds;
  coeff_bounds.resize(degree+1, make_tuple(0,prime));

  string line;
  string nmb;

  while ( getline(stream, line) ) {
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

    if (lbd >= ubd) {
      cerr << "lower bound must be less than or equal to upper one " << ix <<  " " << lbd << " " << ubd << endl;
      throw;
    }

    coeff_bounds[ix] = make_tuple(lbd, ubd);
  }

  return coeff_bounds;
}
