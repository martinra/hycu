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

#include <block_iterator.hh>
#include <iterator_messaging.hh>


using namespace std;


BOOST_AUTO_TEST_CASE( blocks )
{
  map<size_t, tuple<int,int>> blocks
      { {0, make_tuple(2, 4)}
      , {1, make_tuple(5, 9)}
      , {2, make_tuple(0, 1)}
      };
  BlockIterator iter(3, blocks);

  vector<vector<int>> positions;
  for (; !iter.is_end(); iter.step() )
    positions.emplace_back(iter.as_position());
  sort(positions.begin(), positions.end());


  vector<vector<int>> positions_valid = 
       { {2,5,0}, {2,6,0} 
       , {2,7,0}, {2,8,0} 
       , {3,5,0}, {3,6,0} 
       , {3,7,0}, {3,8,0} };

  if ( positions != positions_valid )
    message_positions("blocks positions: ", positions);

}

BOOST_AUTO_TEST_CASE( sets )
{
  map<size_t, vector<int>> sets
      { {0, {5, 7}}
      , {1, {3}}
      , {2, {9, 13}}
      };

  auto iter = BlockIterator(3, {}, sets);


  vector<vector<int>> positions;
  for (; !iter.is_end(); iter.step() )
    positions.push_back(iter.as_position());
  sort(positions.begin(), positions.end());


  vector<vector<int>> positions_valid =
      { {5,3,9}, {5,3,13}
      , {7,3,9}, {7,3,13} };

  if ( positions != positions_valid )
    message_positions("sets positions: ", positions);
}

BOOST_AUTO_TEST_CASE( dependent_sets )
{
  map<size_t, vector<int>> sets
      { {0, {5, 7}}
      };
  map<size_t, tuple<size_t, map<int, vector<int>>>> dependent_sets
      { {1, make_tuple( 0, map<int, vector<int>>{ {0, {2, 4}}
                                                , {1, {3, 6}}
                                                }
                      )}
      };

  auto iter = BlockIterator(2, {}, sets, dependent_sets);


  vector<vector<int>> positions;
  for (; !iter.is_end(); iter.step() )
    positions.push_back(iter.as_position());
  sort(positions.begin(), positions.end());


  vector<vector<int>> positions_valid =
      { {5,2}, {5,4}
      , {7,3}, {7,6} };

  if ( positions != positions_valid )
    message_positions("dependent sets positions: ", positions);
}


BOOST_AUTO_TEST_CASE( blocks_sets_package_position )
{
  map<size_t, tuple<int,int>> blocks
      { {0, make_tuple(2, 10)}
      , {1, make_tuple(5, 7)}
      };
  map<size_t, vector<int>> sets
      { {2, {9, 7}}
      , {3, {3}}
      };

  BlockIterator iter(4, blocks, 3, sets);

  vector<vector<int>> positions;
  vector<vector<tuple<int,int>>> position_blocks;
  for (; !iter.is_end(); iter.step() )
    positions.emplace_back(iter.as_position());
  sort(positions.begin(), positions.end());


  vector<vector<int>> positions_valid
       { {2,5,7,3}, {2,5,9,3} 
       , {2,6,7,3}, {2,6,9,3} 
       , {5,5,7,3}, {5,5,9,3} 
       , {5,6,7,3}, {5,6,9,3} 
       , {8,5,7,3}, {8,5,9,3} 
       , {8,6,7,3}, {8,6,9,3} };

  if ( positions != positions_valid )
    message_positions("blocks sets package positions: ", positions);
}

BOOST_AUTO_TEST_CASE( blocks_sets_package_block )
{
  map<size_t, tuple<int,int>> blocks
      { {0, make_tuple(2, 10)}
      , {1, make_tuple(5, 7)}
      };
  map<size_t, vector<int>> sets
      { {2, {9, 7}}
      , {3, {3}}
      };

  BlockIterator iter(4, blocks, 3, sets);

  vector<vector<tuple<int,int>>> position_blocks;
  for (; !iter.is_end(); iter.step() )
    position_blocks.emplace_back(iter.as_block());
  sort(position_blocks.begin(), position_blocks.end());

  vector<tuple<int,int>> position_blocks_valid
      { make_tuple(2,5)
      , make_tuple(5,6)
      , make_tuple(7,8)
      , make_tuple(3,4) };

  if ( position_blocks[0] != position_blocks_valid )
    message_position_blocks( "blocks sets package first block", position_blocks );


  position_blocks_valid =
      { make_tuple(5,8)   
      , make_tuple(5,6)
      , make_tuple(7,8)
      , make_tuple(3,4) };

  if ( position_blocks[4] != position_blocks_valid )
    message_position_blocks( "blocks sets package fifth block", position_blocks );
}

BOOST_AUTO_TEST_CASE( blocks_package_block_enumerator )
{
  map<size_t, tuple<int,int>> blocks
      { {0, make_tuple(2, 9)}
      , {1, make_tuple(5, 7)}
      , {2, make_tuple(8, 10)}
      };

  BlockIterator iter(3, blocks, 4);

  vector<vector<int>> positions, positions_valid;

  auto iter_sub = iter.as_block_enumerator();
  for (; !iter_sub.is_end(); iter_sub.step())
    positions.emplace_back(iter_sub.as_position());
  sort(positions.begin(), positions.end());

  positions_valid = { {2,5,8}, {3,5,8}, {4,5,8}, {5,5,8} };
  if ( positions != positions_valid )
    message_positions("blocks package block first enumerator: ", positions);


  iter.step(); iter.step(); iter.step();
  positions.clear();
  iter_sub = iter.as_block_enumerator();
  for (; !iter_sub.is_end(); iter_sub.step())
    positions.emplace_back(iter_sub.as_position());
  sort(positions.begin(), positions.end());

  positions_valid = { {6,5,9}, {7,5,9}, {8,5,9} };
  if ( positions != positions_valid )
    message_positions("blocks package block fourth enumerator: ", positions);
}
