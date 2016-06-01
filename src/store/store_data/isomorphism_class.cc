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
#include "store/store_data/isomorphism_class.hh"


using namespace HyCu::StoreData;
using namespace std;


IsomorphismClass::
IsomorphismClass(
    const Curve & curve
    ) :
    curve ( curve )
{
  // this is not implemented for the twists
  if ( curve.prime_power() == 2 ) throw;

  if ( CurveIterator::is_minimal_in_isomorphism_class(curve) )
    this->value.representatives.insert(curve.rhs_coeff_exponents());
}

const IsomorphismClass
IsomorphismClass::
twist()
{
  if ( this->value.representatives.size() > 1 ) {
    cerr << "values of IsomorphismClass stores must have at most one representative" << endl;
    throw;
  }

  return IsomorphismClass(curve.twist());
}

ostream &
HyCu::StoreData::
operator<<(
    ostream & stream,
    const IsomorphismClass::ValueType & value
    )
{
  bool start = true;
  for ( const auto & representative : value.representatives ) {
    if ( !start )
      stream << ";";
    else
      start = false;
    IsomorphismClass::ValueType::insert_representative(stream, representative);
  }

  return stream;
}

ostream &
IsomorphismClass::ValueType::
insert_representative(
    ostream & stream,
    const vector<int> & representative
    )
{
  if ( !representative.empty() ) {
    stream << representative.front();
    for ( size_t ix=0; ix<representative.size(); ++ix )
      stream << "," << representative[ix];
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

  value.representatives.clear();
  while ( true ) {
    IsomorphismClass::ValueType::extract_representative(stream,representative);
    value.representatives.insert(representative);
  
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
extract_representative(
    istream & stream,
    vector<int> & representative
    )
{
  int read_int;
  char delimiter;
  
  representative.clear();
  while ( true ) {
    stream >> read_int;
    representative.push_back(read_int);
  
    delimiter = stream.peek();
    if ( delimiter == ',' ) {
      stream.ignore(1);
      continue;
    }
    else
      return stream;
  }
}
