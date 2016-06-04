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


#ifndef _H_CURVE_FQ
#define _H_CURVE_FQ

#include "curve.hh"


class CurveFq :
  public Curve
{
  public:
    CurveFq(shared_ptr<FqElementTable> table, const vector<fq_nmod_struct*> & poly_coefficients);
    CurveFq(shared_ptr<FqElementTable> table, vector<fq_nmod_struct*> && poly_coefficients);

    ~CurveFq();


    inline
    vector<fq_nmod_struct*>
    rhs_coefficients(
        )
    {
      this->poly_coefficients = move( ((Curve*)this)->rhs_coefficients() );
      return vector<fq_nmod_struct*>(this->poly_coefficients);
    };


    inline
    void
    clear_rhs_coefficients()
    {
      poly_coefficients.clear();
    };

  private:
    vector<fq_nmod_struct*> poly_coefficients; 
};

#endif
