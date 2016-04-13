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
