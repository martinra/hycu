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


using std::cerr;
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
      map<vector<int>, unsigned int> orbits;
      vector<vector<int>> representatives;


      inline
      ValueType()
      {
      };

      inline
      ValueType(
          const vector<vector<int>> & representatives
          ) :
        representatives ( representatives )
      {
      };

      inline
      ValueType(
          vector<vector<int>> && representatives
          ) :
        representatives ( move(representatives) )
      {
      };

      inline
      ValueType(
          const map<vector<int>, unsigned int> & orbits,
          const vector<vector<int>> & representatives
          ) :
        orbits ( orbits ),
        representatives ( representatives )
      {
      };

      inline
      ValueType(
          map<vector<int>, unsigned int> && orbits,
          vector<vector<int>> && representatives
          ) :
        orbits ( move(orbits ) ),
        representatives ( move(representatives) )
      {
      };

      inline
      ValueType(
          const IsomorphismClass & data
          ) :
        orbits ( data.value.orbits ),
        representatives ( data.value.representatives )
      {
      };

      inline
      ValueType(
          IsomorphismClass && data
          ) :
        orbits ( move(data.value.orbits) ),
        representatives ( move(data.value.representatives) )
      {
      };

      static ostream & insert_polynomial(ostream & stream, const vector<int> & polynomial);
      static istream & extract_polynomial(istream & stream, vector<int> & polynomial);
    };

    friend void operator+=(ValueType & lhs, const IsomorphismClass & rhs);


    template<class CurveData>
    inline
    bool
    was_inserted(
        const map<CurveData,IsomorphismClass> & store,
        const Curve & curve
        )
    {
      if ( !this->with_orbits ) {
        cerr << "HyCu::StoreData::IsomorphismClass::was_inserted: no orbit data available" << endl;
        throw;
      }

      const auto & poly_coeff_exponents = curve.rhs_coeff_exponents();

      for ( const auto & store_data : store )
        if ( store_data.second.value.all_orbits.count(poly_coeff_exponents) == 1 )
          return true;

      return false;
    };

  protected:
    ValueType value;

  private:
    shared_ptr<FqElementTable> base_field_table;
    bool with_orbits;

    inline
    IsomorphismClass(
        map<vector<int>, unsigned int> && orbits,
        vector<vector<int>> && representatives
        ) :
      value ( orbits, representatives )
    {};
};


inline
bool
operator==(
    const IsomorphismClass::ValueType & lhs,
    const IsomorphismClass::ValueType & rhs)
{
  return    set<vector<int>>(lhs.representatives.cbegin(), lhs.representatives.cend())
         == set<vector<int>>(rhs.representatives.cbegin(), rhs.representatives.cend());
};

inline
void
operator+=(
    IsomorphismClass::ValueType & lhs,
    const IsomorphismClass::ValueType & rhs
    )
{
  unsigned int lhs_size = lhs.representatives.size();

  for ( const auto & poly : rhs.representatives )
    lhs.representatives.emplace_back(poly);
  lhs.representatives.begin(), lhs.representatives.end();

  for ( const auto & orbit_poly : rhs.orbits )
    lhs.orbits[orbit_poly.first] = orbit_poly.second + lhs_size;
};

inline
void
operator+=(
    IsomorphismClass::ValueType & lhs,
    const IsomorphismClass & rhs
    )
{
  lhs += rhs.value;
};


ostream & operator<<(ostream & stream, const IsomorphismClass::ValueType & value);
istream & operator>>(istream & stream, IsomorphismClass::ValueType & value);

}
}

#endif
