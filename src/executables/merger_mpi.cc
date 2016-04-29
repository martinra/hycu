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


#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#include "mpi/store.hh"


using namespace std;
namespace filesys = boost::filesystem;


int
main(
    int argc,
    char** argv
    )
{
  if (argc < 3) {
    cerr << "Arguments: input_folder, output_file" << endl;
    exit(1);
  }


  filesys::path input(argv[1]);
  if ( !filesys::exists(input) ) {
    cerr << "input folder does not exist" << endl;
    exit(1);
  }
  if ( !filesys::is_directory(input) ) {
    cerr << "input folder is not a directory" << endl;
    exit(1);
  }

  vector<filesys::path> input_files;
  copy( filesys::directory_iterator(input), filesys::directory_iterator(),
        back_inserter(input_files) );


  MPIStore store;
  for ( auto const& input_file : input_files )
    if ( input_file.extension() == ".hycu_unmerged" )
      fstream(input_file.native(), ios_base::in) >> store;


  fstream(argv[2], ios_base::out) << store;
}
