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


#ifndef _H_CURVE
#define _H_CURVE

#include <map>
#include <memory>
#include <vector>
#include <tuple>
#include <flint/fq_nmod_poly.h>
#include <flint/nmod_poly.h>

#include "fq_element_table.hh"
#include "opencl/interface.hh"
#include "reduction_table.hh"


using std::map;
using std::move;
using std::shared_ptr;
using std::vector;
using std::tuple;


class Curve
{
  public:
    Curve(
        shared_ptr<FqElementTable> table,
        const vector<int> & poly_coeff_exponents
        ) :
      table( table ),
      poly_coeff_exponents ( poly_coeff_exponents )
    {
    };

    Curve(
        shared_ptr<FqElementTable> table,
        vector<int> && poly_coeff_exponents
        ) :
      table( table ),
      poly_coeff_exponents ( move(poly_coeff_exponents) )
    {
    };


    inline
    const shared_ptr<FqElementTable>
    base_field_table(
        ) const
    {
      return this->table;
    };

    inline
    unsigned int
    prime(
        ) const
    {
      return this->table->prime;
    };

    inline
    unsigned int
    prime_exponent(
        ) const
    {
      return this->table->prime_exponent;
    };

    inline
    unsigned int
    prime_power(
        ) const
    {
      return this->table->prime_power;
    };

    // todo: this should be unsigned int
    inline
    int
    degree(
        ) const
    {
      return this->poly_coeff_exponents.size() - 1;
    };

    int genus() const;


    inline
    const vector<int> &
    rhs_coeff_exponents(
        ) const
    {
      return this->poly_coeff_exponents;
    };

    vector<int> rhs_coeff_exponents(const ReductionTable & table);


    vector<fq_nmod_struct*> rhs_coefficients() const;

    nmod_poly_struct rhs_nmod_polynomial() const;
    fq_nmod_poly_struct rhs_polynomial() const;

    bool rhs_is_squarefree() const;


    inline vector<unsigned int> rhs_support() const;


    unsigned int discriminant() const;


    // todo: remove static function
    inline
    Curve
    twist(
        ) const
    {
      return Curve(this->table, Curve::_twist_rhs(this->table, this->poly_coeff_exponents));
    };

    static vector<int> _twist_rhs(const shared_ptr<FqElementTable> base_field_table, const vector<int> & poly_coeff_exponents);


    inline
    void
    count(
        const shared_ptr<ReductionTable> table
        )
    {
      this->count(*table);
    };

    void count(ReductionTable & table);

    inline
    bool
    has_counted(
        size_t fx
        ) const
    {
      return (this->nmb_points.find(fx) != this->nmb_points.end());
    };


    inline
    const map<unsigned int, tuple<int,int>> &
    number_of_points(
        ) const
    {
      return this->nmb_points;
    };

    vector<tuple<int,int>> number_of_points(unsigned int max_prime_exponent) const;

    map<unsigned int, int> hasse_weil_offsets() const;
    vector<int> hasse_weil_offsets(unsigned int max_prime_exponent) const;

    vector<int> ramification_type() const;


    friend
    inline
    bool
    operator==(
        const Curve & left,
        const Curve & right
        )
    {
      return    *left.table == *right.table
             && left.poly_coeff_exponents == right.poly_coeff_exponents;
    };

    friend
    inline
    bool
    operator!=(
        const Curve & left,
        const Curve & right
        )
    {
      return !(left == right);
    };

    friend
    inline
    bool
    operator<=(
        const Curve & left,
        const Curve & right
        )
    {
      if ( *left.table != *right.table )
        return *left.table <= *right.table;
    
      return left.poly_coeff_exponents <= right.poly_coeff_exponents;
    };

    friend
    inline
    bool
    operator>=(
        const Curve & left,
        const Curve & right
        )
    {
      if ( *left.table != *right.table )
        return *left.table >= *right.table;
    
      return left.poly_coeff_exponents >= right.poly_coeff_exponents;
    };
    
    friend
    inline
    bool
    operator<(
        const Curve & left,
        const Curve & right
        )
    {
      return !(left >= right);
    };

    friend
    inline
    bool
    operator>(
        const Curve & left,
        const Curve & right
        )
    {
      return !(left <= right);
    };


    friend ostream& operator<<(ostream &stream, const Curve & curve);

  protected:
    Curve(
        shared_ptr<FqElementTable> table
        ) :
      table ( table )
    {
    };


    const shared_ptr<FqElementTable> table;
    vector<int> poly_coeff_exponents;

    map<unsigned int, tuple<int,int>> nmb_points;

  private:
    void count_opencl(ReductionTable & table, const vector<int> & poly_coeff_exponents);
    void count_cpu(const ReductionTable & table, const vector<int> & poly_coeff_exponents);
};

#endif
