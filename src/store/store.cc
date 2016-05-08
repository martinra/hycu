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

#include <sstream>
#include <string>

#include "store/store.hh"


using namespace std;


namespace std
{
  bool
  less<curve_data>::
  operator()(
      const curve_data & lhs,
      const curve_data & rhs
      ) const
  {
    if ( lhs.ramification_type < rhs.ramification_type )
      return true;
    else if ( lhs.ramification_type == rhs.ramification_type )
      if ( lhs.hasse_weil_offsets < rhs.hasse_weil_offsets )
        return true;

    return false;
  };
}

ostream &
operator<<(
    ostream & stream,
    const curve_data & data
    )
{
  if ( !data.ramification_type.empty() ) {
    stream << data.ramification_type.front();
    for (size_t ix=1; ix<data.ramification_type.size(); ++ix)
      stream << "," << data.ramification_type[ix];
  }

  stream << ";";

  if ( !data.hasse_weil_offsets.empty() ) {
    stream << data.hasse_weil_offsets.front();
    for (size_t ix=1; ix<data.hasse_weil_offsets.size(); ++ix)
      stream << "," << data.hasse_weil_offsets[ix];
  }

  return stream;
}

istream &
operator>>(
    istream & stream,
    curve_data & data
    )
{
  int read_int;
  char delimiter;
  
  data.ramification_type.clear();
  data.hasse_weil_offsets.clear();

  while ( true ) {
    stream >> read_int;
    data.ramification_type.push_back(read_int);
  
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
    data.hasse_weil_offsets.push_back(read_int);
  
    delimiter = stream.peek();
    if ( delimiter == ',' ) {
      stream.ignore(1);
      continue;
    }
    else
      return stream;
  }
}

string
Store::
output_file_name(
    const MPIConfigNode & config,
    const vuu_block & block
    )
{
  stringstream output_name(ios_base::out);
  output_name << "store";

  output_name << "__prime_power_" << pow(config.prime, config.prime_exponent);
  output_name << "__coeff_exponent_bounds";
  for ( auto bds : block )
    output_name << "__" << get<0>(bds) << "_" << get<1>(bds);

  output_name << ".hycu_unmerged";

  return (config.result_path / path(output_name.str())).native();
}

curve_data
Store::
twisted_curve_data(
    const curve_data & data
    )
{
  vector<int> twisted_hasse_weil_offsets;
  twisted_hasse_weil_offsets.reserve(data.hasse_weil_offsets.size());
  for ( int offset : data.hasse_weil_offsets )
    twisted_hasse_weil_offsets.push_back(-offset);

  return { data.ramification_type, twisted_hasse_weil_offsets };
}
