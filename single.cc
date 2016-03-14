#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <tuple>

#include <count.hh>




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


  vector<int> poly_coeffs;
  for (int ix=2; ix < argc; ++ix)
    poly_coeffs.push_back(atoi(argv[ix]));

  int degree = poly_coeffs.size() - 1;
  int genus;
  if (degree % 2 == 0)
    genus = (degree - 2)/2;
  else
    genus = (degree - 1)/2;


  auto opencl = OpenCLInterface();

  vector<ReductionTableFq> tables;
  for (int px = 1; px <= genus; ++px)
    tables.emplace_back(ReductionTableFq(prime,px, opencl));

  Curve(prime, poly_coeffs).count_verbose(tables[1]);

  auto nmb_points = Curve(prime, poly_coeffs).isogeny_nmb_points(tables, opencl);

  for (auto pts : nmb_points)
    cout << get<0>(pts) << " " << get<1>(pts) << ";  ";
  cout << endl;

  if (genus=2) {
    int a1 = prime + 1 - (get<0>(nmb_points[0]) + get<1>(nmb_points[0]));
    int a2 = prime*prime + 1 - (get<0>(nmb_points[1]) + get<1>(nmb_points[1]));

    int c3 = -a1;
    int c2 = (a1*a1-a2)/2;
//    while (c3 < 0) c3 += prime;
//    while (c2 < 0) c2 += prime;
//    c3 %= prime;
//    c2 %= prime;

    cout << "x^4 + " << c3 << "x^3 + " << c2 << "x^2 + ..." << endl;
  }

  return 0;
}
// test:
// make single && ./single 5 1 2 3 1 1 0 4
// should have 40 points over F_25
