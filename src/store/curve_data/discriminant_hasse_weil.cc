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


#include "store/curve_data/discriminant_hasse_weil.hh"


using namespace std;
using namespace HyCu::CurveData;


DiscriminantHasseWeil
DiscriminantHasseWeil::
twist()
{
  unsigned int nonsquare_generator_power =
    base_field_table->power_coset_representatives(2)[1];
  unsigned int nonsquare_power = base_field_table->at_nmod(
     this->base_field_table->reduce_generator_exponent(
       (2*this->degree-2) * nonsquare_generator_power ) );

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

  return DiscriminantHasseWeil(
      base_field_table, this->degree, this->prime,
      nonsquare_power * this->value.discriminant % this->prime,
      twisted_hasse_weil_offsets );
}

ostream &
HyCu::CurveData::
operator<<(
    ostream & stream,
    const DiscriminantHasseWeil::ValueType & value
    )
{
  stream << value.discriminant;

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
    DiscriminantHasseWeil::ValueType & value
    )
{
  int read_int;
  char delimiter;
  
  value.hasse_weil_offsets.clear();

  stream >> value.discriminant;

  delimiter = stream.peek();
  if ( delimiter == ';' ) {
    stream.ignore(1);
  }
  else
    return stream;

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
