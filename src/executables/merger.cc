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
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <vector>

#include "store/curve_data.hh"
#include "store/file_store.hh"
#include "store/store.hh"
#include "store/store_data.hh"


namespace filesys = boost::filesystem;
namespace popt = boost::program_options;
using namespace std;
using popt::value;


template<class Store>
int
merge(
    vector<filesys::path> input_files,
    filesys::path record_output_file,
    filesys::path store_output_file
    );


int
main(
    int argc,
    char** argv
    )
{
  popt::options_description visible_options("Available options"), all_options;
  popt::positional_options_description positional_options;

  visible_options.add_options()
    ( "help,h", "show help message" )
//    ( "store-type", value<string>(),
//      "the type of the store; c: Count" )
    ( "input-path", value<string>(),
      "path to the input folder" )
    ( "output-file", value<string>(),
      "path to the output file omitting the extension" );

  positional_options//.add("store-type", 1)
                    .add("input-path", 1)
                    .add("output-file", 1);

  popt::variables_map options_map;
  popt::store( popt::command_line_parser(argc, argv)
                 .options(visible_options)
                 .positional(positional_options)
                 .run(),
               options_map );
  popt::notify(options_map);


  if ( options_map.count("help") ) {
    cerr << visible_options;
    return 0;
  }


//  if ( !options_map.count("store-type") ) {
//   cerr << "store-type has to be set" << endl;
//   return 1;
//  }
  if ( !options_map.count("input-path") ) {
   cerr << "input-path has to be set" << endl;
   return 1;
  }
  if ( !options_map.count("output-file") ) {
   cerr << "output-file has to be set" << endl;
   return 1;
  }
  

  filesys::path input(options_map["input-path"].as<string>());
  if ( !filesys::exists(input) ) {
    cerr << "input-path does not exist" << endl;
    return 1;
  }
  if ( !filesys::is_directory(input) ) {
    cerr << "input-path is not a folder" << endl;
    return 1;
  }

  vector<filesys::path> input_files;
  copy( filesys::directory_iterator(input), filesys::directory_iterator(),
        back_inserter(input_files) );


  filesys::path record_output_file(
      options_map["output-file"].as<string>() + FileStore::record_extension );
  filesys::path store_output_file(
      options_map["output-file"].as<string>() + FileStore::store_extension );

//  string store_type = options_map["store-type"].as<string>();
  if ( true ) // store_type == "c" )
    return merge<Store<HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>>
      (input_files, record_output_file, store_output_file);
//  else {
//      cerr << "undefined store-type: " << options_map["store-type"].as<string>() << endl;
//      return 1;
//  }
}

template<class Store>
int
merge(
    vector<filesys::path> input_files,
    filesys::path record_output_file,
    filesys::path store_output_file
    )
{
  set<vuu_block> record;
  Store store;
  for ( auto const& record_file : input_files ) {
    if (    !filesys::is_regular_file(record_file)
         || record_file.extension() != FileStore::record_extension )
      continue;

    filesys::path store_file(record_file);
    store_file.replace_extension( FileStore::store_extension );
    if ( !filesys::is_regular_file(store_file) ) {
      cerr << "found record file " << record_file.filename()
           << ", but no corresponding store file" << endl;
      return 1;
    }

    FileStore::extract( fstream(record_file.native(), ios_base::in), record );
    store.extract( fstream(store_file.native(), ios_base::in) );
  }

  // save record second as a witness to successful writing
  store.insert( fstream(store_output_file.native(), ios_base::out) );
  FileStore::insert( fstream(record_output_file.native(), ios_base::out), record );

  return 0;
}
