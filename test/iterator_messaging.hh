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


#ifndef _H_ITERATOR_MESSAGING
#define _H_ITERATOR_MESSAGING

#include <vector>
#include <string>
#include <tuple>


using std::string;
using std::vector;
using std::tuple;


void message_positions( string && description, const vector<vector<unsigned int>> & positions );
void message_position_blocks( string && description, const vector<vector<tuple<unsigned int,unsigned int>>> & blocks );

#endif
