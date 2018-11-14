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


#include "flint/fmpz.h"
#include <iostream>
#include <set>
#include <string>


using std::istream;
using std::ostream;
using std::move;
using std::set;
using std::string;


namespace HyCu
{
namespace StoreData
{

class Count
{
  public:
    inline Count(const Curve & curve)
    {
      CurveIterator::multiplicity( value.counter,
          curve.prime(), curve.prime_power(),
          curve.rhs_support() );
    };
    
    inline const Count twist() { return *this; };

    inline bool operator==(const Count & rhs) const
    {
      return this->value.counter == rhs.value.counter;
    };

    struct ValueType
    {
      fmpz_t counter;

      ValueType()
      {
        fmpz_init(this->counter);
      };

      ValueType(unsigned int counter)
      {
        fmpz_init(this->counter);
        fmpz_set_ui(this->counter, counter);
      };

      ValueType(const Count & count)
      {
        fmpz_init(this->counter);
        fmpz_set(this->counter, count.value.counter);
      };

      ValueType(const string & str);

      ~ValueType()
      {
        fmpz_clear(this->counter);
      };
    };

    friend void operator+=(ValueType & lhs, const Count & rhs);

  protected:
    ValueType value;

  private:
    Count(unsigned long int counter) : value ( counter ) {};
    Count(const fmpz * counter) : value ( counter ) {};
    Count(fmpz * && counter) : value ( counter ) {};
};


inline bool operator==(const Count::ValueType & lhs, const Count::ValueType & rhs)
{
  return fmpz_equal(lhs.counter, rhs.counter) == 1;
};

inline void operator+=(Count::ValueType & lhs, const Count::ValueType & rhs)
{
  fmpz_add(lhs.counter, lhs.counter, rhs.counter);
};

inline void operator+=(Count::ValueType & lhs, const Count & rhs)
{
  fmpz_add(lhs.counter, lhs.counter, rhs.value.counter);
};


inline ostream & operator<<(ostream & stream, const Count::ValueType & value)
{
  char * c_str = fmpz_get_str(NULL, 10, value.counter);
  stream << string(c_str);
  flint_free(c_str);

  return stream;
};

}
}

#endif
