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


#include <iostream>
#include <isogeny_representative_store.hh>


void
IsogenyRepresentativeStore::
register_curve(
    const Curve & curve
    )
{
  auto store_key =
    make_tuple( curve.ramification_type(),
                curve.hasse_weil_offsets(curve.prime_exponent() * curve.genus()) );
  auto store_it = this->store.find(store_key);
  if ( store_it == this->store.end() )
    this->store[store_key] = curve.poly_coeff_exponents;
}

ostream &
operator<<(
    ostream & stream,
    const IsogenyRepresentativeStore & store
    )
{
  for ( auto & store_it : store.store ) {
    auto & ramifications = get<0>(store_it.first);
    auto & hasse_weil_offsets = get<1>(store_it.first);
    auto & poly_coeff_exponents = store_it.second;


    if ( !ramifications.empty() ) {
      stream << ramifications.front();
      for (size_t ix=1; ix<ramifications.size(); ++ix)
        stream << "," << ramifications[ix];
    }
    stream << ";";

    if ( !hasse_weil_offsets.empty() ) {
      stream << hasse_weil_offsets.front();
      for (size_t ix=1; ix<hasse_weil_offsets.size(); ++ix)
        stream << "," << hasse_weil_offsets[ix];
    }
    stream << ":";

    if ( !poly_coeff_exponents.empty() ) {
      stream << poly_coeff_exponents.front();
      for (size_t ix=1; ix<poly_coeff_exponents.size(); ++ix)
        stream << "," << poly_coeff_exponents[ix];
    }

    stream << endl;
  }

  return stream;
}

istream &
operator>>(
    istream & stream,
    IsogenyRepresentativeStore & store
    )
{
  int read_int;
  vector<int> ramifications, hasse_weil_offsets, poly_coeff_exponents;

  while ( !stream.eof() && (char)stream.peek() != ';' ) {
    stream.ignore(1,',') >> read_int;
    ramifications.push_back(read_int);
  }

  if ( stream.eof() ) {
    cerr << "isogeny_representative_store.operator>>: unexpected end of file (ramification)" << endl;
    throw;
  }
  stream.ignore(1,';');

  while ( !stream.eof() && (char)stream.peek() != ':' ) {
    stream.ignore(1,',') >> read_int;
    hasse_weil_offsets.push_back(read_int);
  }

  if ( stream.eof() ) {
    cerr << "isogeny_representative_store.operator>>: unexpected end of file (hasse_weil_offsets)" << endl;
    throw;
  }
  stream.ignore(1,':');

  while ( !stream.eof() && (char)stream.peek() != '\n' ) {
    stream.ignore(1,',') >> read_int;
    poly_coeff_exponents.push_back(read_int);
  }


  auto store_key = make_tuple(ramifications, hasse_weil_offsets);
  auto store_it = store.store.find(store_key);
  if ( store_it == store.store.end() )
    store.store[store_key] = poly_coeff_exponents;


  return stream;
}
