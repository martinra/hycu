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


#include "store/curve_data/explicit_ramification_hasse_weil.hh"


using namespace std;
using namespace HyCu::CurveData;


bool
less<ExplicitRamificationHasseWeil::ValueType>::
operator()(
    const ExplicitRamificationHasseWeil::ValueType & lhs,
    const ExplicitRamificationHasseWeil::ValueType & rhs
    ) const
{
  if ( lhs.ramification_type < rhs.ramification_type )
    return true;
  else if ( lhs.ramification_type == rhs.ramification_type )
    if ( lhs.hasse_weil_offsets < rhs.hasse_weil_offsets )
      return true;

  return false;
};

ExplicitRamificationHasseWeil
ExplicitRamificationHasseWeil::
twist()
{
  vector<int> twisted_hasse_weil_offsets;
  twisted_hasse_weil_offsets.reserve(this->value.hasse_weil_offsets.size());
  bool odd = true;
  for ( int offset : this->value.hasse_weil_offsets ) {
    if ( odd )
      twisted_hasse_weil_offsets.push_back(-offset);
    else
      twisted_hasse_weil_offsets.push_back(offset);
    odd = !odd;
  }

  return ExplicitRamificationHasseWeil(this->value.ramification_type, twisted_hasse_weil_offsets);
}

ostream &
HyCu::CurveData::
operator<<(
    ostream & stream,
    const ExplicitRamificationHasseWeil::ValueType & value
    )
{
  if ( !value.ramification_type.empty() ) {
    stream << value.ramification_type.front();
    for (size_t ix=1; ix<value.ramification_type.size(); ++ix)
      stream << "," << value.ramification_type[ix];
  }

  stream << ";";

  if ( !value.hasse_weil_offsets.empty() ) {
    stream << value.hasse_weil_offsets.front();
    for (size_t ix=1; ix<value.hasse_weil_offsets.size(); ++ix)
      stream << "," << value.hasse_weil_offsets[ix];
  }

  return stream;
}

istream &
HyCu::CurveData::
operator>>(
    istream & stream,
    ExplicitRamificationHasseWeil::ValueType & value
    )
{
  int read_int;
  char delimiter;
  
  value.ramification_type.clear();
  value.hasse_weil_offsets.clear();

  while ( true ) {
    stream >> read_int;
    value.ramification_type.push_back(read_int);
  
    delimiter = stream.peek();
    if ( delimiter == ',' ) {
      stream.ignore(1);
      continue;
    }
    else if ( delimiter == ';' ) {
      stream.ignore(1);
      break;
    }
    else
      return stream;
  }

  while ( true ) {
    stream >> read_int;
    value.hasse_weil_offsets.push_back(read_int);
  
    delimiter = stream.peek();
    if ( delimiter == ',' ) {
      stream.ignore(1);
      continue;
    }
    else
      return stream;
  }
}
