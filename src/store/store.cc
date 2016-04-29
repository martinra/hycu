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
