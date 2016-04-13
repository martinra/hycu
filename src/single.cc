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

    
  cout << "genus: " << curve->genus() << ", degree: " << curve->degree();
  cout << " / F_" << curve->prime_power() << endl;

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

  if ( curve->genus() == 2 ) {
    auto hasse_weil_offsets = curve->hasse_weil_offsets(); 

    int a1 = hasse_weil_offsets[1];
    int a2 = hasse_weil_offsets[2];

    int c3 = -a1;
    int c2 = (a1*a1-a2)/2;

    cout << "x^4 + " << c3 << "x^3 + " << c2 << "x^2 + ..." << endl;
  }

  return 0;
}

