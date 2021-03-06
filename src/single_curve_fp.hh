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


#ifndef _H_SINGLE_CURVE_FP
#define _H_SINGLE_CURVE_FP

#include <memory>
#include <vector>

#include "curve.hh"


using std::shared_ptr;
using std::vector;

enum SingleCurveCountImplementation
{
  SingleCurveCountImplementationCPU,
  SingleCurveCountImplementationOpenCL,
  SingleCurveCountImplementationNaiveNMod,
  SingleCurveCountImplementationNaiveZech
};

shared_ptr<Curve>
single_curve_fp(
    unsigned int prime,
    vector<unsigned int> poly_coeffs,
    SingleCurveCountImplementation implementation
    );

#endif
