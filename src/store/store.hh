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


#ifndef _H_STORE_CURVE_DATA
#define _H_STORE_CURVE_DATA

#include <string>
#include <vector>

#include "block_iterator.hh"
#include "config/config_node.hh"
#include "curve.hh"


using std::istream;
using std::ostream;
using std::string;
using std::vector;


typedef struct {
  vector<int> ramification_type;
  vector<int> hasse_weil_offsets;
} curve_data;


namespace std
{
  template<> struct
  less<curve_data>
  {
    bool operator()(const curve_data & lhs, const curve_data & rhs) const;
  };
}

ostream & operator<<(ostream & stream, const curve_data & data);
istream & operator>>(istream & stream, curve_data & data);


class Store
{
  public:
    unsigned int
    inline
    moduli_multiplicity(
      const Curve & curve
      )
    {
      return CurveIterator::multiplicity( curve.prime(), curve.prime_power(),
                                          curve.rhs_support() );
    };

    curve_data twisted_curve_data(const curve_data & curve_data);

    string output_file_name(const MPIConfigNode & config, const vuu_block & block);

  protected:
    Store() = default;
};

#endif
