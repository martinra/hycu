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


#ifndef _H_STORE_STORE_DATA_COUNT
#define _H_STORE_STORE_DATA_COUNT


#include <iostream>
#include <set>

#include "curve_iterator.hh"


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
    inline
    Count(
        const Curve & curve
        )
    {
      this->value.counter = CurveIterator::multiplicity(curve);
    };
    
    inline
    const Count
    twist()
    {
      return *this;
    };

    struct ValueType
    {
      unsigned int counter;


      ValueType(
          ) :
        counter ( 0 )
      {
      };

      ValueType(
          unsigned int counter
          ) :
        counter ( counter )
      {
      };

      ValueType(
          const Count & count
          ) :
        counter ( count.value.counter )
      {
      };

      ValueType(
          Count && count
          ) :
        counter ( count.value.counter )
      {
      };
    };

    friend void operator+=(ValueType & lhs, const Count & rhs);

  protected:
    ValueType value;
};


inline
bool
operator==(
    const Count::ValueType & lhs,
    const Count::ValueType & rhs
    )
{
  return lhs.counter == rhs.counter;
};

inline
void
operator+=(
    Count::ValueType & lhs,
    const Count::ValueType & rhs
    )
{
  lhs.counter += rhs.counter;
};

inline
void
operator+=(
    Count::ValueType & lhs,
    const Count & rhs
    )
{
  lhs.counter += rhs.value.counter;
};


inline
ostream & operator<<(
    ostream & stream,
    const Count::ValueType & value
    )
{
  return stream << value.counter;
};

inline
istream & operator>>(
    istream & stream,
    Count::ValueType & value
    )
{
  return stream >> value.counter;
};

}
}

#endif
