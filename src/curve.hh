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
using std::shared_ptr;
using std::vector;
using std::tuple;


class Curve
{
  public:
    Curve(shared_ptr<FqElementTable> table, const vector<int> poly_coeff_exponents);

    unsigned int inline prime() const { return this->table->prime; };
    unsigned int inline prime_exponent() const { return this->table->prime_exponent; };
    unsigned int inline prime_power() const { return this->table->prime_power; };

    int inline degree() const { return this->poly_coeff_exponents.size() - 1; };
    int genus() const;
    vector<int> inline rhs_coeff_exponents() const { return this->poly_coeff_exponents; };
    vector<unsigned int> rhs_support() const;

    bool has_squarefree_rhs();
    nmod_poly_struct rhs_nmod_polynomial() const;
    fq_nmod_poly_struct rhs_polynomial() const;

    vector<int> convert_poly_coeff_exponents(const ReductionTable & table);

    void count(ReductionTable & table);
    void inline count(const shared_ptr<ReductionTable> table)
    {
      this->count(*table);
    };
    void count_naive_nmod(int prime_exponent);
    void count_naive_zech(int prime_exponent);

    bool has_counted(size_t fx) const { return (this->nmb_points.find(fx) != this->nmb_points.end()); };

    const map<unsigned int, tuple<int,int>> & number_of_points() const { return this->nmb_points; };
    vector<tuple<int,int>> number_of_points(unsigned int max_prime_exponent) const;

    unsigned int max_prime_exponent() const;
    map<unsigned int, int> hasse_weil_offsets() const;
    vector<int> hasse_weil_offsets(unsigned int max_prime_exponent) const;

    vector<int> ramification_type() const;

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
