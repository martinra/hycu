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


#ifndef _H_STORE_CURVE_DATA_DISCRIMINANT_HASSE_WEIL
#define _H_STORE_CURVE_DATA_DISCRIMINANT_HASSE_WEIL

#include <vector>
#include <iostream>

#include "curve.hh"


using std::cerr;
using std::endl;
using std::istream;
using std::ostream;


template<class CurveData, class StoreData> class Store;


namespace HyCu
{
namespace CurveData
{

class DiscriminantHasseWeil
{
  public:
    DiscriminantHasseWeil(const Curve & curve) :
      base_field_table ( curve.base_field_table() ),
      degree ( curve.degree() ),
      prime ( curve.prime() ),
      value( curve.discriminant() )
    {
      if ( curve.prime_exponent() != 1 ) {
        cerr << "HyCu::CurveData::DiscriminantHasseWeil: implemented only for curves over prime fields" << endl;
        throw;
      }
    };


    struct ValueType
    {
      unsigned int discriminant;


      inline
      ValueType(
          ) :
        discriminant ( 0 )
      {
      };

      inline
      ValueType(
          unsigned int discriminant
          ) :
        discriminant ( discriminant )
      {
      };

      explicit inline ValueType( const DiscriminantHasseWeil & data ) :
        ValueType ( data.value.discriminant ) {};
      explicit inline ValueType( const DiscriminantHasseWeil && data ) :
        ValueType ( data.value.discriminant ) {};
    };

    DiscriminantHasseWeil twist();

    inline ValueType as_value() { return ValueType( *this ); };

  private:
    DiscriminantHasseWeil(
        const shared_ptr<FqElementTable> base_field_table,
        unsigned int degree,
        unsigned int prime,
        unsigned int discriminant
        ) :
      base_field_table ( base_field_table ),
      degree ( degree ),
      prime ( prime),
      value ( discriminant )
    { 
    };

    const shared_ptr<FqElementTable> base_field_table;
    const unsigned int degree;
    const unsigned int prime;

    ValueType value;
};


inline
bool
operator==(
    const DiscriminantHasseWeil::ValueType & lhs,
    const DiscriminantHasseWeil::ValueType & rhs
    )
{
  return lhs.discriminant == rhs.discriminant;
};

ostream & operator<<(ostream & stream, const DiscriminantHasseWeil::ValueType & value);
istream & operator>>(istream & stream, DiscriminantHasseWeil::ValueType & value);

}
}


namespace std
{
  using namespace HyCu::CurveData;


  template<> struct
  less<DiscriminantHasseWeil::ValueType>
  {
    inline
    bool
    operator()(
        const DiscriminantHasseWeil::ValueType & lhs,
        const DiscriminantHasseWeil::ValueType & rhs
        ) const
    {
      return lhs.discriminant < rhs.discriminant;
    };
  };
}

#endif
