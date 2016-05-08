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
  curve_data curve_data =
    { curve.ramification_type(),
      curve.hasse_weil_offsets(curve.prime_exponent() * curve.genus()) };
  store_count_representative_data store_data =
    {this->moduli_multiplicity(curve), curve.rhs_coeff_exponents()};

  auto store_it = this->store.find(curve_data);
  if ( store_it == this->store.end() ) {
    store[curve_data] = store_data;
    store[this->twisted_curve_data(curve_data)] = this->twisted_store_data(store_data, curve);
  }
  else {
    store_it->second.count += store_data.count;
    store[this->twisted_curve_data(curve_data)].count += this->twisted_store_data(store_data, curve).count;
  }
}

store_count_representative_data
StoreCountRepresentative::
twisted_store_data(
    const store_count_representative_data & data,
    const Curve & curve
    )
{
  auto prime_power_pred = curve.prime_power() - 1;
  auto zero_index = prime_power_pred;
  unsigned int nonsquare = 1;

  vector<int> twisted_poly_coeff_exponents;
  twisted_poly_coeff_exponents.reserve(data.representative_poly_coeff_exponents.size());
  for ( int coeff : data.representative_poly_coeff_exponents ) {
    if ( coeff == zero_index )
      twisted_poly_coeff_exponents.push_back(coeff);
    else
      twisted_poly_coeff_exponents.push_back((coeff + nonsquare) % prime_power_pred);
  }

  return { data.count, twisted_poly_coeff_exponents };
}


ostream &
operator<<(
    ostream & stream,
    const StoreCountRepresentative & store
    )
{
  for ( auto & store_it : store.store )
    stream << store_it.first << ":" << store_it.second << endl;

  return stream;
}

istream &
operator>>(
    istream & stream,
    StoreCountRepresentative & store
    )
{
  char delimiter;

  curve_data curve_data;
  store_count_representative_data store_data;

  stream.peek();
  while ( !stream.eof() ) {
    stream >> curve_data;
    delimiter = stream.peek();
    if ( delimiter != ':' ) {
      cerr << "operator>> StoreCountRepresentative cannot extract: character after curve data is " << delimiter << endl;
      throw;
    }
    stream.ignore(1);

    stream >> store_data;
    delimiter = stream.peek();
    if ( delimiter != '\n' ) {
      cerr << "operator>> StoreCountRepresentative cannot extract: character after store data is " << delimiter <<  endl;
      throw;
    }
    stream.ignore(1);

    auto store_it = store.store.find(curve_data);
    if ( store_it == store.store.end() )
      store.store[curve_data] = store_data;
    else
      store.store[curve_data].count += store_data.count;

    stream.peek();
  }

  return stream;
}

ostream &
operator<<(
    ostream & stream,
    const store_count_representative_data & data
    )
{
  stream << data.count;

  stream << ";";

  if ( !data.representative_poly_coeff_exponents.empty() ) {
    stream << data.representative_poly_coeff_exponents.front();
    for (size_t ix=1; ix<data.representative_poly_coeff_exponents.size(); ++ix)
      stream << "," << data.representative_poly_coeff_exponents[ix];
  }

  return stream;
}

istream &
operator>>(
    istream & stream,
    store_count_representative_data & data
    )
{
  int read_int;
  char delimiter;
  
  data.representative_poly_coeff_exponents.clear();

  stream >> data.count;
  delimiter = stream.peek();
  if ( delimiter == ';' )
    stream.ignore(1);
  else
    return stream;


  while ( true ) {
    stream >> read_int;
    data.representative_poly_coeff_exponents.push_back(read_int);
  
    delimiter = stream.peek();
    if ( delimiter == ',' ) {
      stream.ignore(1);
      continue;
    }
    else
      return stream;
  }
}
