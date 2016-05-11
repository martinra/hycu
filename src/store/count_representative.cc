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


#include <fstream>

#include "store/count_representative.hh"


using namespace std;


void
StoreCountRepresentative::
register_curve(
    const Curve & curve
    )
{
  curve_data key =
    { curve.ramification_type(),
      curve.hasse_weil_offsets(curve.prime_exponent() * curve.genus()) };
  store_count_representative_data store_data =
    {this->moduli_multiplicity(curve), curve.rhs_coeff_exponents()};

  auto store_it = this->store.find(key);
  if ( store_it == this->store.end() ) {
    store[key] = store_data;

    auto twisted_key = this->twisted_curve_data(key);
    if ( twisted_key == key )
      store[key].count += store_data.count;
    else
      store[twisted_key] = this->twisted_store_data(store_data, curve);
  }
  else {
    store_it->second.count += store_data.count;
    store[this->twisted_curve_data(key)].count += store_data.count;
  }
}

