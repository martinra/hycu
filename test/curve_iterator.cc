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


#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <tuple>

#include "curve.hh"
#include "curve_iterator.hh"
#include "fq_element_table.hh"
#include "iterator_messaging.hh"


using namespace std;


BOOST_AUTO_TEST_CASE( blocks_f13_g2 )
{
  unsigned int prime = 13;

  auto table = make_shared<FqElementTable>(prime, 1);
  CurveIterator iter(*table, 2, 2500);

  fmpz_t total_nmb;
  fmpz_init(total_nmb);
  fmpz_zero(total_nmb);

  fmpz_t tmp;
  fmpz_init(tmp);

  for (; !iter.is_end(); iter.step() ) {
    BlockIterator block_iter(iter.as_block());
    for (; !block_iter.is_end(); block_iter.step() ) {
      Curve curve(table, block_iter.as_position());
      if ( !curve.has_squarefree_rhs() ) continue;
      CurveIterator::multiplicity(tmp, prime, prime, curve.rhs_support());
      fmpz_add(total_nmb, total_nmb, tmp);
      // twist
      fmpz_add(total_nmb, total_nmb, tmp);
    }
  }

  fmpz_t total_nmb_cmp;
  fmpz_init(total_nmb_cmp);
  fmpz_zero(total_nmb_cmp);

  fmpz_set_ui(tmp, prime);
  fmpz_pow_ui(tmp, tmp, 7);
  fmpz_add(total_nmb_cmp, total_nmb_cmp, tmp);

  fmpz_set_ui(tmp, prime);
  fmpz_pow_ui(tmp, tmp, 4);
  fmpz_add(total_nmb_cmp, total_nmb_cmp, tmp);

  fmpz_set_ui(tmp, prime);
  fmpz_pow_ui(tmp, tmp, 6);
  fmpz_sub(total_nmb_cmp, total_nmb_cmp, tmp);

  fmpz_set_ui(tmp, prime);
  fmpz_pow_ui(tmp, tmp, 5);
  fmpz_sub(total_nmb_cmp, total_nmb_cmp, tmp);

  BOOST_CHECK( fmpz_equal(total_nmb, total_nmb_cmp) == 1 );

  fmpz_clear(total_nmb);
  fmpz_clear(total_nmb_cmp);
  fmpz_clear(tmp);
}
