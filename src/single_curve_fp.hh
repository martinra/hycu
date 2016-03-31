#ifndef _H_SINGLE_CURVE_FP
#define _H_SINGLE_CURVE_FP

#include <memory>
#include <vector>

#include <curve.hh>


using namespace std;


unique_ptr<Curve> single_curve_fp(unsigned int prime, vector<unsigned int> poly_coeffs);

#endif
