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

  auto store_it = this->store.find(curve_data);
  if ( store_it == this->store.end() )
    // fixme: take stabilizers into account
    store[curve_data] = {1, curve.rhs_coeff_exponents()};
  else
    // fixme: take stabilizers into account
    ++store_it->second.count;
}

ostream &
operator<<(
    ostream & stream,
    const StoreCountRepresentative & store
    )
{
  for ( auto & store_it : store.store ) {
    const auto & curve_data = store_it.first;
    const auto & store_data = store_it.second;

    if ( !curve_data.ramification_type.empty() ) {
      stream << curve_data.ramification_type.front();
      for (size_t ix=1; ix<curve_data.ramification_type.size(); ++ix)
        stream << "," << curve_data.ramification_type[ix];
    }
    stream << ";";

    if ( !curve_data.hasse_weil_offsets.empty() ) {
      stream << curve_data.hasse_weil_offsets.front();
      for (size_t ix=1; ix<curve_data.hasse_weil_offsets.size(); ++ix)
        stream << "," << curve_data.hasse_weil_offsets[ix];
    }
    stream << ":";

    stream << store_data.count;
    stream << ";";

    if ( !store_data.representative_poly_coeff_exponents.empty() ) {
      stream << store_data.representative_poly_coeff_exponents.front();
      for (size_t ix=1; ix<store_data.representative_poly_coeff_exponents.size(); ++ix)
        stream << "," << store_data.representative_poly_coeff_exponents[ix];
    }

    stream << endl;
  }

  return stream;
}

istream &
operator>>(
    istream & stream,
    StoreCountRepresentative & store
    )
{
  curve_data curve_data;
  store_count_representative_data store_data;
  int read_int;


  // read ramification_type
  while ( !stream.eof() && (char)stream.peek() != ';' ) {
    stream.ignore(1,',') >> read_int;
    curve_data.ramification_type.push_back(read_int);
  }

  if ( stream.eof() ) {
    cerr << "isogeny_representative_store.operator>>: "
         << "unexpected end of file (ramification_type)" << endl;
    throw;
  }
  stream.ignore(1,';');

  // read hasse_weil_offsets
  while ( !stream.eof() && (char)stream.peek() != ':' ) {
    stream.ignore(1,',') >> read_int;
    curve_data.hasse_weil_offsets.push_back(read_int);
  }

  if ( stream.eof() ) {
    cerr << "isogeny_representative_store.operator>>: "
         << "unexpected end of file (hasse_weil_offsets)" << endl;
    throw;
  }
  stream.ignore(1,':');

  // read count
  if ( stream.eof() || (char)stream.peek() == ';' ) {
    cerr << "isogeny_representative_store.operator>>: "
         << "unexpected end of file (count)" << endl;
    throw;
  }

  stream >> store_data.count;
  stream.ignore(1,';');

  // read representative_poly_coeff_exponents
  while ( !stream.eof() && (char)stream.peek() != '\n' ) {
    stream.ignore(1,',') >> read_int;
    store_data.representative_poly_coeff_exponents.push_back(read_int);
  }


  auto store_it = store.store.find(curve_data);
  if ( store_it == store.store.end() )
    store.store[curve_data] = store_data;
  else
    store.store[curve_data].count += store_data.count;


  return stream;
}
