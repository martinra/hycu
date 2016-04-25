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


#ifndef _H_MPI_STORE
#define _H_MPI_STORE

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "block_iterator.hh"
#include "curve.hh"
#include "mpi/config_node.hh"
#include "mpi/store.hh"


using std::istream;
using std::map;
using std::vector;
using std::string;


typedef struct {
  vector<int> ramification_type;
  vector<int> hasse_weil_offsets;
} curve_data;

typedef struct {
  unsigned int count;
  vector<int> representative_poly_coeff_exponents;
} store_data;


namespace std
{
  template<> struct
  less<curve_data>
  {
    bool
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
  };
}


class MPIStore
{
  public:
    MPIStore(const MPIConfigNode & config, const vuu_block & block) :
      config ( config ), block ( block ) {};
  
    void register_curve(const Curve & curve);

    void write_to_file();
  
    friend ostream & operator<<(ostream & stream, const MPIStore & store);
    friend istream & operator>>(istream & stream, MPIStore & store);

  protected:
    MPIConfigNode config;
    vuu_block block;

    map<curve_data, store_data> store;

    string output_file_name();
};

#endif
