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
#include <iostream>
#include <vector>
#include <tuple>
#include <yaml-cpp/yaml.h>

#include "curve_iterator.hh"
#include "config/config_node.hh"
#include "store/store_factory.hh"
#include "worker_pool/standalone.hh"


namespace popt = boost::program_options;
using namespace std;
using boost::filesystem::current_path;
using boost::filesystem::path;
using boost::filesystem::is_directory;
using boost::filesystem::is_regular_file;
using popt::value;


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
    ( "config-file,c", value<string>(),
      "path to configuration file" )
    ( "output-path", value<string>(),
      "path to the output root" )
    ( "nmb-threads,n", value<int>()->default_value(-1),
      "number of working threads" )
    ( "nmb-threads-per-gpu,g", value<unsigned int>()->default_value(1),
      "number of threads assigned per GPU" );

  positional_options.add("config-file", 1)
                    .add("output-path", 1);

  popt::variables_map options_map;
  popt::store( popt::command_line_parser(argc, argv)
                 .options(visible_options)
                 .positional(positional_options)
                 .run(),
               options_map );
  popt::notify(options_map);


  if ( options_map.count("help") ) {
    cerr << visible_options;
    exit(0);
  }

  if ( !options_map.count("config-file") ) {
    cerr << "configuration file required" << endl;
    exit(1);
  }

  if ( !options_map.count("output-path") ) {
    cerr << "output path required" << endl;
    exit(1);
  }


  path output_path(options_map["output-path"].as<string>());
  if ( !( is_directory(output_path) || create_directories(output_path) ) ) {
    cerr << "could not create output path" << endl;
    exit(1);
  }


  path config_file(options_map["config-file"].as<string>());
  if ( !is_regular_file(config_file) ) {
    cerr << "could not find configuration file or it is not a regular file" << endl;
    exit(1);
  }
  auto config_yaml = YAML::LoadFile(config_file.native());


  StoreType store_type;
  if ( !config_yaml["StoreType"] )
    store_type = StoreType::EC;
  else {
    auto store_type_str = config_yaml["StoreType"].as<string>();
    if ( store_type_str == "EC" )
      store_type = StoreType::EC;
    else {
      cerr << "Invalid store type in configuration file" << endl;
      exit(1);
    }
  }


  vector<ConfigNode> config;
  if ( !config_yaml["Moduli"] )
    config.emplace_back(config_yaml.as<ConfigNode>());
  else
    for ( const auto & node : config_yaml["Moduli"] )
      config.emplace_back(node.as<ConfigNode>());


  StandaloneWorkerPool
    worker_pool( store_type,
        options_map["nmb-threads"].as<int>(),
        options_map["nmb-threads-per-gpu"].as<unsigned int>() );

  for ( auto & node : config ) {
    node.prepend_output_path(canonical(output_path,current_path()));
    if ( !node.verify() ) {
      cerr << "Incorrect configuration node:" << endl << node;
      exit(1);
    }
    worker_pool.update_config(node);

    FqElementTable enumeration_table(node.prime, node.prime_exponent);
    CurveIterator iter(enumeration_table, node.genus, node.package_size);
    for (; !iter.is_end(); iter.step() )
      worker_pool.assign(iter.as_block());
  }

  return 0;
}
