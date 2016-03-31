#include <map>

#include <opencl_interface.hh>
#include <reduction_table.hh>

#include <single_curve_fp.hh>


unique_ptr<Curve>
single_curve_fp(
    unsigned int prime,
    vector<unsigned int> poly_coeffs
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


  auto opencl = make_shared<OpenCLInterface>();
  auto curve = make_unique<Curve>(enumeration_table, poly_coeff_exponents);

  for ( size_t fx=curve->genus(); fx>curve->genus()/2; --fx ) {
    ReductionTable reduction_table(prime, fx, opencl);
    curve->count(reduction_table);
  }

  return curve;
}
