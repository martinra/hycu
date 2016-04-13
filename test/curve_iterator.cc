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

#include <curve_iterator.hh>
#include <fq_element_table.hh>
#include <iterator_messaging.hh>


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
       { {0,0,1}, {0,0,2}
       , {1,0,1}, {1,0,2}
       , {2,0,1}, {2,0,2}
       };
  if ( positions_deg2 != positions_deg2_valid )
    message_positions("genus 1 degree 2 curves / F_3: ", positions_deg2);

  vector<vector<int>> positions_deg1_valid = 
       { {0,1}, {0,2}
       };
  if ( positions_deg1 != positions_deg1_valid )
    message_positions("genus 1 degree 1 curves / F_3: ", positions_deg1);
}

