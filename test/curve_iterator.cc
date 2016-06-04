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
#include <set>
#include <tuple>

#include "curve.hh"
#include "curve_iterator.hh"
#include "fq_element_table.hh"
#include "iterator_messaging.hh"


using namespace std;


BOOST_AUTO_TEST_CASE( enumerate_f2_g1 )
{
  FqElementTable table(2, 1);
  CurveIterator iter(table, 1, 1);

  vector<vector<int>> positions_deg4;
  vector<vector<int>> positions_deg3;
  for (; !iter.is_end(); iter.step() ) {
    auto position = iter.as_position();
    for_each( position.begin(), position.end(),
              [&table](int &p){ p = table.at_nmod(p); } );
    if ( position.size() == 5 )
      positions_deg4.emplace_back(position);
    else
      positions_deg3.emplace_back(position);
  }
  sort(positions_deg4.begin(), positions_deg4.end());
  sort(positions_deg3.begin(), positions_deg3.end());


  vector<vector<int>> positions_deg4_valid = 
       { {0,0,0,0,1}, {0,0,1,0,1}, {0,1,0,0,1}, {0,1,1,0,1}
       , {1,0,0,0,1}, {1,0,1,0,1}, {1,1,0,0,1}, {1,1,1,0,1}
       };
  if ( positions_deg4 != positions_deg4_valid )
    message_positions("genus 1 degree 4 curves / F_2: ", positions_deg4);

  vector<vector<int>> positions_deg3_valid = 
       { {0,0,0,1}, {0,1,0,1}
       , {1,0,0,1}, {1,1,0,1}
       };
  if ( positions_deg3 != positions_deg3_valid )
    message_positions("genus 1 degree 3 curves / F_2: ", positions_deg3);
}


BOOST_AUTO_TEST_CASE( enumerate_f3_g0 )
{
  FqElementTable table(3, 1);
  CurveIterator iter(table, 0, 1);

  vector<vector<int>> positions_deg2;
  vector<vector<int>> positions_deg1;
  for (; !iter.is_end(); iter.step() ) {
    auto position = iter.as_position();
    for_each( position.begin(), position.end(),
              [&table](int &p){ p = table.at_nmod(p); } );
    if ( position.size() == 3 )
      positions_deg2.emplace_back(position);
    else
      positions_deg1.emplace_back(position);
  }
  sort(positions_deg2.begin(), positions_deg2.end());
  sort(positions_deg1.begin(), positions_deg1.end());


  vector<vector<int>> positions_deg2_valid = 
       { {0,0,1}
       , {1,0,1}, {1,0,2}
       };
  if ( positions_deg2 != positions_deg2_valid )
    message_positions("genus 1 degree 2 curves / F_3: ", positions_deg2);

  vector<vector<int>> positions_deg1_valid = 
       { {0,1},
       };
  if ( positions_deg1 != positions_deg1_valid )
    message_positions("genus 1 degree 1 curves / F_3: ", positions_deg1);
}

BOOST_AUTO_TEST_CASE( is_reduced_f5_g1 )
{
  unsigned int prime = 5;
  unsigned int genus = 1;
  auto table = make_shared<FqElementTable>(prime,1);


  set<vector<int>> curves_itered;
  for ( CurveIterator iter(*table,genus,1); !iter.is_end(); iter.step() ) {
    Curve curve(table, iter.as_position());
    curves_itered.insert(curve.rhs_coeff_exponents());
    curves_itered.insert(CurveIterator::reduce(curve.twist()).rhs_coeff_exponents());
  }

  vuu_block lower_degree_blocks;
  for ( size_t ix=0; ix < 2*genus + 1; ++ix )
    lower_degree_blocks.push_back(table->block_complete());
  lower_degree_blocks.push_back(table->block_non_zero());
  BlockIterator lower_degree_block_iter(lower_degree_blocks);

  vuu_block higher_degree_blocks;
  for ( size_t ix=0; ix < 2*genus + 2; ++ix )
    higher_degree_blocks.push_back(table->block_complete());
  higher_degree_blocks.push_back(table->block_non_zero());
  BlockIterator higher_degree_block_iter(higher_degree_blocks);


  set<vector<int>> curves_reduced;
  for ( ; !lower_degree_block_iter.is_end(); lower_degree_block_iter.step() ) {
    auto poly_coeff_exponents = lower_degree_block_iter.as_position();
    if ( CurveIterator::is_reduced(Curve(table, poly_coeff_exponents)) )
      curves_reduced.insert(poly_coeff_exponents);
  }
  for ( ; !higher_degree_block_iter.is_end(); higher_degree_block_iter.step() ) {
    auto poly_coeff_exponents = higher_degree_block_iter.as_position();
    if ( CurveIterator::is_reduced(Curve(table, poly_coeff_exponents)) )
      curves_reduced.insert(poly_coeff_exponents);
  }


  if ( curves_itered != curves_reduced ) {
    vector<vector<int>> curves_diff;

    for ( const auto & v : curves_reduced )
      if ( curves_itered.count(v) == 0 )
       curves_diff.push_back(v);

    curves_diff.push_back(vector<int>{-1});

    for ( const auto & v : curves_itered )
      if ( curves_reduced.count(v) == 0 )
       curves_diff.push_back(v);

    message_positions( "(un)reduced genus 1 curves / F_5: ", curves_diff );
  }
}

BOOST_AUTO_TEST_CASE( reduce_f5_g1 )
{
  unsigned int prime = 5;
  unsigned int genus = 1;
  auto table = make_shared<FqElementTable>(prime,1);


  vuu_block lower_degree_blocks;
  for ( size_t ix=0; ix < 2*genus + 1; ++ix )
    lower_degree_blocks.push_back(table->block_complete());
  lower_degree_blocks.push_back(table->block_non_zero());
  BlockIterator lower_degree_block_iter(lower_degree_blocks);

  vuu_block higher_degree_blocks;
  for ( size_t ix=0; ix < 2*genus + 2; ++ix )
    higher_degree_blocks.push_back(table->block_complete());
  higher_degree_blocks.push_back(table->block_non_zero());
  BlockIterator higher_degree_block_iter(higher_degree_blocks);


  for ( ; !lower_degree_block_iter.is_end(); lower_degree_block_iter.step() )
    if ( !CurveIterator::is_reduced(
            CurveIterator::reduce(
              Curve(table, lower_degree_block_iter.as_position()) ) ) )
      message_positions( "unreduced genus 1 curve / F_5: ",
                         vector<vector<int>>{ lower_degree_block_iter.as_position() } );
  for ( ; !higher_degree_block_iter.is_end(); higher_degree_block_iter.step() )
    if ( !CurveIterator::is_reduced(
            CurveIterator::reduce(
              Curve(table, higher_degree_block_iter.as_position()) ) ) )
      message_positions( "unreduced genus 1 curve / F_5: ",
                         vector<vector<int>>{ higher_degree_block_iter.as_position() } );
}

