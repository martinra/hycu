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


#ifndef _H_STORE_CURVE_DATA_HASSE_WEIL
#define _H_STORE_CURVE_DATA_HASSE_WEIL

#include <vector>
#include <iostream>

#include "curve.hh"


using std::cerr;
using std::endl;
using std::istream;
using std::move;
using std::ostream;
using std::vector;


template<class CurveData, class StoreData> class Store;


namespace HyCu
{
namespace CurveData
{

class HasseWeil
{
  public:
    HasseWeil(
        const Curve & curve
        ) :
      HasseWeil( curve.hasse_weil_offsets(curve.prime_exponent() * curve.genus()) )
    {
      // we use this asumption in the implementation of twist. Check the general formulas.
      if ( curve.prime_exponent() != 1 ) {
        cerr << "HasseWeil: only implemented for prime_exponent 1" << endl;
        throw;
      }
    };
    
    HasseWeil twist();


    struct ValueType
    {
      vector<int> hasse_weil_offsets;
  

      inline
      ValueType()
      {
      };

      inline
      ValueType(
          const vector<int> & hasse_weil_offsets
          ) :
        hasse_weil_offsets ( hasse_weil_offsets )
      {
      };

      inline
      ValueType(
          vector<int> && hasse_weil_offsets
          ) :
        hasse_weil_offsets ( hasse_weil_offsets )
      {
      };

      explicit
      inline
      ValueType(
          const HasseWeil & data
          ) :
        ValueType ( data.value.hasse_weil_offsets )
      {
      };

      explicit
      inline
      ValueType(
          HasseWeil && data
          ) :
        ValueType ( data.value.hasse_weil_offsets )
      {
      };
    };

    inline
    ValueType
    as_value()
    {
      return ValueType( *this );
    };


    template<class StoreData>
    inline
    bool
    was_inserted(
        const map<HasseWeil,StoreData> & store,
        const Curve & curve
        )
    {
      return false;
    };

  private:
    HasseWeil(
        const vector<int> & hasse_weil_offsets
        ) :
      value ( ValueType(hasse_weil_offsets) )
    {
    };

    HasseWeil(
        vector<int> && hasse_weil_offsets
        ) :
      value( ValueType(hasse_weil_offsets) )
    {
    };


    ValueType value;
};


inline
bool
operator==(
    const HasseWeil::ValueType & lhs,
    const HasseWeil::ValueType & rhs
    )
{
  return lhs.hasse_weil_offsets == rhs.hasse_weil_offsets;
};

ostream & operator<<(ostream & stream, const HasseWeil::ValueType & value);
istream & operator>>(istream & stream, HasseWeil::ValueType & value);

}
}


namespace std
{
  using namespace HyCu::CurveData;


  template<> struct
  less<HasseWeil::ValueType>
  {
    inline
    bool
    operator()(
        const HasseWeil::ValueType & lhs,
        const HasseWeil::ValueType & rhs
        ) const
    {
      return lhs.hasse_weil_offsets < rhs.hasse_weil_offsets;
    };
  };
}

#endif
