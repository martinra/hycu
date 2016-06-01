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


#ifndef _H_STORE_STORE_DATA_ISOMORPHISM_CLASS
#define _H_STORE_STORE_DATA_ISOMORPHISM_CLASS


#include <iostream>
#include <set>


using std::istream;
using std::ostream;
using std::move;
using std::set;


namespace HyCu
{
namespace StoreData
{

class IsomorphismClass
{
  public:
    IsomorphismClass(const Curve & curve);
    
    const IsomorphismClass twist();

    struct ValueType
    {
      set<vector<int>> representatives;

      ValueType() {};
      ValueType(const set<vector<int>> & representatives) : representatives ( representatives ) {};
      ValueType(set<vector<int>> && representatives) : representatives ( move(representatives) ) {};

      ValueType(const IsomorphismClass & data) : representatives ( data.value.representatives ) {};
      ValueType(IsomorphismClass && data) : representatives ( data.value.representatives ) {};

      static ostream & insert_representative(ostream & stream, const vector<int> & representative);
      static istream & extract_representative(istream & stream, vector<int> & representative);
    };

    friend void operator+=(ValueType & lhs, const IsomorphismClass & rhs);

  protected:
    ValueType value;

  private:
    IsomorphismClass(unsigned int prime_power_pred, set<vector<int>> representatives) :
      prime_power_pred ( prime_power_pred ), value ( representatives ) {};
  
    unsigned int prime_power_pred;
};


inline void operator+=(IsomorphismClass::ValueType & lhs, const IsomorphismClass::ValueType & rhs)
{
  lhs.representatives.insert(rhs.representatives.begin(), rhs.representatives.end());
};

inline void operator+=(IsomorphismClass::ValueType & lhs, const IsomorphismClass & rhs) { lhs += rhs.value; };


ostream & operator<<(ostream & stream, const IsomorphismClass::ValueType & value);
istream & operator>>(istream & stream, IsomorphismClass::ValueType & value);

}
}

#endif
