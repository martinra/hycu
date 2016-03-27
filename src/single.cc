#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <tuple>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>


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
  auto enumeration_table = make_shared<FqElementTable>(prime, 1);

  unsigned int gen = enumeration_table->at_nmod(1);
  unsigned int a = 1;
  map<unsigned int, unsigned int> fp_to_exponent;
  fp_to_exponent[0] = prime-1;
  for (unsigned int ex=0; ex<prime-1; ++ex) {
    fp_to_exponent[a] = ex;
    a = (a*gen) % (prime-1);
  }

  vector<int> poly_coeff_exponents;
  for (int ix=2; ix < argc; ++ix)
    poly_coeff_exponents.push_back(fp_to_exponent[atoi(argv[ix])]);


  int degree = poly_coeff_exponents.size() - 1;
  int genus;
  if (degree % 2 == 0)
    genus = (degree - 2)/2;
  else
    genus = (degree - 1)/2;


  auto opencl = make_shared<OpenCLInterface>();
  ReductionTable reduction_table(prime, genus, opencl);
  auto curve = Curve(enumeration_table, poly_coeff_exponents);
  curve.count(reduction_table);

    
  for ( auto & pts : curve.number_of_points(genus) )
    cout << get<0>(pts) << " " << get<1>(pts) << ";  ";
  cout << endl;

  if ( genus=2 ) {
    auto hasse_weil_offsets = curve.hasse_weil_offsets(); 

    int a1 = hasse_weil_offsets[0];
    int a2 = hasse_weil_offsets[1];

    int c3 = -a1;
    int c2 = (a1*a1-a2)/2;

    cout << "x^4 + " << c3 << "x^3 + " << c2 << "x^2 + ..." << endl;
  }


  return 0;
}
// test:
// make single && ./single 5 1 2 3 1 1 0 4
// should have 40 points over F_25
