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


#ifndef _H_STORE_COUNT_REPRESENTATIVE
#define _H_STORE_COUNT_REPRESENTATIVE

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "block_iterator.hh"
#include "config/config_node.hh"
#include "curve.hh"
#include "store/count_representative.hh"
#include "store/store.hh"


using std::istream;
using std::map;
using std::vector;
using std::string;


typedef struct {
  unsigned int count;
  vector<int> representative_poly_coeff_exponents;
} store_count_representative_data;


class StoreCountRepresentative :
  public Store
{
  public:
    void register_curve(const Curve & curve);

    store_count_representative_data twisted_store_data(const store_count_representative_data & data, const Curve & curve);

    friend ostream & operator<<(ostream & stream, const StoreCountRepresentative & store);
    friend istream & operator>>(istream & stream, StoreCountRepresentative & store);

  private:
    map<curve_data, store_count_representative_data> store;
};

#endif
