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


#ifndef _H_STORE_REPRESENTATIVE
#define _H_STORE_REPRESENTATIVE

#include <iostream>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "curve.hh"
#include "store/store.hh"


using std::map;
using std::shared_ptr;
using std::tuple;
using std::vector;
using std::istream;


typedef struct {
  vector<int> representative_poly_coeff_exponents;
} store_representative_data;


class StoreRepresentative :
  public Store
{
  public:
    void register_curve(const Curve & curve);

    friend ostream & operator<<(ostream & stream, const StoreRepresentative & store);
    friend istream & operator>>(istream & stream, StoreRepresentative & store);

  private:
    map<curve_data, store_representative_data> store;
};

#endif
