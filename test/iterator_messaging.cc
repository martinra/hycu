#include <boost/test/unit_test.hpp>

#include <sstream>

#include <iterator_messaging.hh>


using namespace std;


void
message_positions(
    string && description,
    const vector<vector<int>> & positions
    )
{
  stringstream out;
  for ( auto position : positions ) {
    for ( auto p : position )
      out << p << " ";
    out << " ;  ";
  }
  BOOST_FAIL( description << out.str() );
}

void
message_position_blocks(
    string && description,
    const vector<vector<tuple<int,int>>> & blocks
    )
{
  stringstream out;
  for ( auto block : blocks ) {
    for ( auto b : block)
      out << "(" << get<0>(b) << " " << get<1>(b) << ")" << " ";
    out << " ;  ";
  }
  BOOST_FAIL( description << out.str() );
}

