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

#include "curve_fq.hh"


using namespace std;


CurveFq::
CurveFq(
    shared_ptr<FqElementTable> table,
    const vector<fq_nmod_struct*> & poly_coefficients
    ) :
  Curve( table )
{
  this->poly_coefficients = poly_coefficients;

  this->poly_coeff_exponents.reserve(poly_coefficients.size());
  for ( auto c : poly_coefficients )
    this->poly_coeff_exponents.push_back(table->generator_power(c));
}

CurveFq::
CurveFq(
    shared_ptr<FqElementTable> table,
    vector<fq_nmod_struct*> && poly_coefficients
    ) :
  Curve( table )
{
  this->poly_coefficients = move(poly_coefficients);

  this->poly_coeff_exponents.reserve(this->poly_coefficients.size());
  for ( auto c : this->poly_coefficients )
    this->poly_coeff_exponents.push_back(table->generator_power(c));
}

CurveFq::
~CurveFq()
{
  for ( auto c : this->poly_coefficients )
    fq_nmod_clear(c, this->table->fq_nmod_ctx());
}
