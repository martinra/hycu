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

#include "curve.hh"
#include "curve_iterator.hh"
#include "store/store_data/isomorphism_class.hh"


using namespace HyCu::StoreData;
using namespace std;


IsomorphismClass::
IsomorphismClass(
    const Curve & curve
    ) :
    base_field_table ( curve.base_field_table() )
{
  const auto & orbit = CurveIterator::orbit(curve, this->value.orbits);

  this->value.representatives.push_back(*orbit.cbegin());
  for ( const auto & poly : orbit )
    this->value.orbits[poly] = 0;
}

const IsomorphismClass
IsomorphismClass::
twist()
{
  vector<set<vector<int>>>
    twisted_orbit_sets(this->value.representatives.size());

  for ( const auto & orbit_poly : this->value.orbits )
    twisted_orbit_sets[orbit_poly.second].emplace(
      Curve::_twist_rhs(this->base_field_table, orbit_poly.first) );

  map<vector<int>, unsigned int> twisted_orbits;
  vector<vector<int>> twisted_representatives;
  for ( unsigned int ix=0; ix<twisted_orbit_sets.size(); ++ix ) {
    const auto & twisted_orbit = twisted_orbit_sets[ix];
    twisted_representatives.push_back(*twisted_orbit.cbegin());
    for ( const auto & poly : twisted_orbit )
      twisted_orbits[poly] = ix; 
  }

  return IsomorphismClass( move(twisted_orbits),
                           move(twisted_representatives) );
}

ostream &
HyCu::StoreData::
operator<<(
    ostream & stream,
    const IsomorphismClass::ValueType & value
    )
{
  bool start = true;
  for ( const auto & representative: value.representatives) {
    if ( !start )
      stream << ";";
    else
      start = false;
    IsomorphismClass::ValueType::insert_polynomial(stream, representative);
  }

  return stream;
}

ostream &
IsomorphismClass::ValueType::
insert_polynomial(
    ostream & stream,
    const vector<int> & polynomial
    )
{
  if ( !polynomial.empty() ) {
    stream << polynomial.front();
    for ( size_t ix=1; ix<polynomial.size(); ++ix )
      stream << "," << polynomial[ix];
  }

  return stream;
}

istream &
HyCu::StoreData::
operator>>(
    istream & stream,
    IsomorphismClass::ValueType & value
    )
{
  vector<int> representative;
  char delimiter;

  value.orbits.clear();
  value.representatives.clear();
  while ( true ) {
    IsomorphismClass::ValueType::extract_polynomial(stream,representative);
    value.representatives.push_back(representative);
  
    delimiter = stream.peek();
    if ( delimiter == ';' ) {
      stream.ignore(1);
      continue;
    }
    else
      return stream;
  }
}

istream &
IsomorphismClass::ValueType::
extract_polynomial(
    istream & stream,
    vector<int> & polynomial 
    )
{
  int read_int;
  char delimiter;
  
  polynomial.clear();
  while ( true ) {
    stream >> read_int;
    polynomial.push_back(read_int);
  
    delimiter = stream.peek();
    if ( delimiter == ',' ) {
      stream.ignore(1);
      continue;
    }
    else
      return stream;
  }
}
