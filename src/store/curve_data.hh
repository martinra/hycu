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

class ExplicitRamificationHasseWeil
{
  public:
    ExplicitRamificationHasseWeil(const Curve & curve) :
      ExplicitRamificationHasseWeil(
          curve.ramification_type(),
          curve.hasse_weil_offsets(curve.prime_exponent() * curve.genus()) )
    {
      // todo: we use this asumption in the implementation of twist. Check the general formulas.
      if ( curve.prime_exponent() != 1 ) {
        cerr << "ExplicitRamificationHasseWeil: only implemented for prime_exponent 1" << endl;
        throw;
      }
    };
    
    ExplicitRamificationHasseWeil twist();

    struct ValueType
    {
      vector<int> ramification_type;
      vector<int> hasse_weil_offsets;
  

      ValueType() {};
      ValueType(const vector<int> & ramification_type, const vector<int> & hasse_weil_offsets) :
        ramification_type ( ramification_type ), hasse_weil_offsets ( hasse_weil_offsets ) {};
      ValueType(vector<int> && ramification_type, vector<int> && hasse_weil_offsets) :
        ramification_type ( move(ramification_type) ), hasse_weil_offsets ( move(hasse_weil_offsets) ) {};

      explicit inline ValueType(const ExplicitRamificationHasseWeil & data) :
        ValueType(data.value.ramification_type, data.value.hasse_weil_offsets) {};
      explicit inline ValueType(ExplicitRamificationHasseWeil && data) :
        ValueType(data.value.ramification_type, data.value.hasse_weil_offsets) {};

      explicit ValueType(const string & str);
    };

    inline ValueType as_value() { return ValueType( *this ); };

  private:
    ExplicitRamificationHasseWeil(const vector<int> & ramification_type, const vector<int> & hasse_weil_offsets) :
      value ( ValueType(ramification_type, hasse_weil_offsets) ) {};
    ExplicitRamificationHasseWeil(vector<int> && ramification_type, vector<int> && hasse_weil_offsets) :
      value( ValueType(ramification_type, hasse_weil_offsets) ) {};

    ValueType value;
};


inline
bool
operator==(
    const ExplicitRamificationHasseWeil::ValueType & lhs,
    const ExplicitRamificationHasseWeil::ValueType & rhs
    )
{
  return (  lhs.ramification_type == rhs.ramification_type
         && lhs.hasse_weil_offsets == rhs.hasse_weil_offsets );
};

ostream & operator<<(ostream & stream, const ExplicitRamificationHasseWeil::ValueType & value);

}
}


namespace std
{
  using namespace HyCu::CurveData;


  template<> struct
  less<ExplicitRamificationHasseWeil::ValueType>
  {
    bool operator()(const ExplicitRamificationHasseWeil::ValueType & lhs,
                    const ExplicitRamificationHasseWeil::ValueType & rhs) const;
  };
}

#endif
