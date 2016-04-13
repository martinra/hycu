#include <fstream>
#include <iostream>

#include <curve.hh>
#include <single_curve_fp.hh>


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

  auto curve = single_curve_fp(prime, poly_coeffs);

    
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


  return 0;
}

