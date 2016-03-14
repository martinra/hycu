#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <tuple>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>

using namespace std;


vector<tuple<int,int>> read_coefficient_bounds( int prime, int genus, istream& stream);


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

  int genus = atoi(argv[2]);
  if (genus < 0) throw;

  fstream input(argv[3], ios_base::in);
  fstream output(argv[4], ios_base::out);


  auto opencl = OpenCLInterface();

  vector<ReductionTable> tables;
  for (int px = 0; px < genus; ++px)
    tables.emplace_back(ReductionTable(prime,px, opencl));


  CurveCounter(prime, read_coefficient_bounds(prime, genus, input))
    .count( [&output](auto poly_coeffs, auto nmb_points)
            {
               for (auto c : poly_coeffs) output << c << " ";
               output << ": ";
               for (auto pts : nmb_points) output << get<0>(pts) << " " << get<1>(pts) << " ";
               output << endl;
            },
            tables, opencl );

  return 0;
}

vector<tuple<int,int>>
read_coefficient_bounds(
    int prime,
    int genus,
    istream& stream
    )
{
  vector<tuple<int,int>> coeff_bounds;
  coeff_bounds.resize(2*genus + 3, make_tuple(0,prime));

  string line;
  string nmb;

  while ( getline(stream, line) ) {
    stringstream line_stream(line);

    getline(line_stream, nmb, ' ');
    int ix = atoi(nmb.c_str());

    if (ix > 2*genus + 3) {
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
