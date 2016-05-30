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


#ifndef _H_STORE_STORE_DATA
#define _H_STORE_STORE_DATA


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

class Count
{
  public:
    inline Count(const Curve & curve)
    {
      this->value.counter = CurveIterator::multiplicity(curve);
    };
    
    inline const Count twist() { return *this; };

    inline bool operator==(const Count & rhs) const
    {
      return this->value.counter == rhs.value.counter;
    };

    struct ValueType
    {
      unsigned int counter;

      ValueType() : counter ( 0 ) {};
      ValueType(unsigned int counter) : counter ( counter ) {};

      ValueType(const Count & count) : counter ( count.value.counter ) {};
      ValueType(Count && count) : counter ( count.value.counter ) {};
    };

    friend void operator+=(ValueType & lhs, const Count & rhs);

  protected:
    ValueType value;

  private:
    Count(unsigned int counter) : value ( counter ) {};
};


inline bool operator==(const Count::ValueType & lhs, const Count::ValueType & rhs)
{
  return lhs.counter == rhs.counter;
};

inline void operator+=(Count::ValueType & lhs, const Count::ValueType & rhs)
{
  lhs.counter += rhs.counter;
};

inline void operator+=(Count::ValueType & lhs, const Count & rhs)
{
  lhs.counter += rhs.value.counter;
};


inline ostream & operator<<(ostream & stream, const Count::ValueType & value)
{
  return stream << value.counter;
};

inline istream & operator>>(istream & stream, Count::ValueType & value)
{
  return stream >> value.counter;
};



class Representative
{
  public:
    Representative(const Curve & curve);
    
    const Representative twist();

    struct ValueType
    {
      set<vector<int>> representatives;

      ValueType() {};
      ValueType(const set<vector<int>> & representatives) : representatives ( representatives ) {};
      ValueType(set<vector<int>> && representatives) : representatives ( move(representatives) ) {};

      ValueType(const Representative & data) : representatives ( data.value.representatives ) {};
      ValueType(Representative && data) : representatives ( data.value.representatives ) {};

      static ostream & insert_representative(ostream & stream, const vector<int> & representative);
      static istream & extract_representative(istream & stream, vector<int> & representative);
    };

    friend void operator+=(ValueType & lhs, const Representative & rhs);

  protected:
    ValueType value;

  private:
    Representative(unsigned int prime_power_pred, set<vector<int>> representatives) :
      prime_power_pred ( prime_power_pred ), value ( representatives ) {};
  
    unsigned int prime_power_pred;
};


inline void operator+=(Representative::ValueType & lhs, const Representative::ValueType & rhs)
{
  lhs.representatives.insert(rhs.representatives.begin(), rhs.representatives.end());
};

inline void operator+=(Representative::ValueType & lhs, const Representative & rhs) { lhs += rhs.value; };


ostream & operator<<(ostream & stream, const Representative::ValueType & value);
istream & operator>>(istream & stream, Representative::ValueType & value);

}
}

#endif
