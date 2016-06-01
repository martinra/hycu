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

#include "curve_iterator.hh"
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
    Curve(shared_ptr<FqElementTable> table, const vector<int> & poly_coeff_exponents) :
      table( table ), poly_coeff_exponents ( poly_coeff_exponents ) {};
    Curve(shared_ptr<FqElementTable> table, vector<int> && poly_coeff_exponents) :
      table( table ), poly_coeff_exponents ( move(poly_coeff_exponents) ) {};
    Curve(shared_ptr<FqElementTable> table, const vector<fq_nmod_struct*> & poly_coefficients);
    Curve(shared_ptr<FqElementTable> table, vector<fq_nmod_struct*> && poly_coefficients);


    inline const shared_ptr<FqElementTable> base_field_table() const
    {
      return this->table;
    };
    unsigned int inline prime() const { return this->table->prime; };
    unsigned int inline prime_exponent() const { return this->table->prime_exponent; };
    unsigned int inline prime_power() const { return this->table->prime_power; };

    int inline degree() const { return this->poly_coeff_exponents.size() - 1; };
    int genus() const;

    inline vector<int> rhs_coeff_exponents() const { return this->poly_coeff_exponents; };
    vector<fq_nmod_struct*> rhs_coefficients() const
    {
      vector<fq_nmod_struct*> fq_rhs;
      fq_rhs.reserve(this->degree()+1);

      for ( int e : this->rhs_coeff_exponents() ) {
        auto fq_coeff = new fq_nmod_struct;
        fq_nmod_init(fq_coeff, this->table->fq_ctx);
        fq_nmod_set(fq_coeff, this->table->at(e), this->table->fq_ctx);
        fq_rhs.push_back(fq_coeff);
      } 

      return fq_rhs;
    };

    inline vector<unsigned int> rhs_support() const
    {
      return Curve::_support(this->table, this->poly_coeff_exponents);
    }

    static vector<unsigned int> _support(shared_ptr<FqElementTable> table, vector<int> poly_coeff_exponents);

    bool has_squarefree_rhs();
    nmod_poly_struct rhs_nmod_polynomial() const;
    fq_nmod_poly_struct rhs_polynomial() const;

    vector<int> convert_poly_coeff_exponents(const ReductionTable & table);

    unsigned int discriminant() const;

    Curve twist() const;


    void count(ReductionTable & table);
    void inline count(const shared_ptr<ReductionTable> table)
    {
      this->count(*table);
    };

    bool has_counted(size_t fx) const { return (this->nmb_points.find(fx) != this->nmb_points.end()); };

    const map<unsigned int, tuple<int,int>> & number_of_points() const { return this->nmb_points; };
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
    const shared_ptr<FqElementTable> table;
    vector<int> poly_coeff_exponents;

    map<unsigned int, tuple<int,int>> nmb_points;

  private:
    void count_opencl(ReductionTable & table, const vector<int> & poly_coeff_exponents);
    void count_cpu(const ReductionTable & table, const vector<int> & poly_coeff_exponents);
};

#endif
