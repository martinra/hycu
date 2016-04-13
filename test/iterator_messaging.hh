#ifndef _H_ITERATOR_MESSAGING
#define _H_ITERATOR_MESSAGING

#include <vector>
#include <string>
#include <tuple>


using std::string;
using std::vector;
using std::tuple;


void message_positions( string && description, const vector<vector<int>> & positions );
void message_position_blocks( string && description, const vector<vector<tuple<int,int>>> & blocks );

#endif
