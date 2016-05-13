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


#include <yaml-cpp/yaml.h>

#include "config/config_node.hh"
#include "curve_iterator.hh"
#include "executables/mpi/master.hh"
#include "fq_element_table.hh"
#include "store/store_factory.hh"
#include "worker_pool/mpi.hh"


namespace mpi = boost::mpi;
using namespace std;


int
main_master(
    int argc,
    char** argv,
    shared_ptr<mpi::communicator> mpi_world
    )
{
  if (argc != 2) {
    cerr << "One argument, the configuration file, is needed" << endl;
    exit(1);
  }

  auto config_yaml = YAML::LoadFile(argv[1]);


  StoreType store_type;
  if ( !config_yaml["StoreType"] )
    store_type = StoreType::EC;
  else {
    auto store_type_str = config_yaml["StoreType"].as<string>();
    if ( store_type_str == "EC" )
      store_type = StoreType::EC;
    else if ( store_type_str == "ER" )
      store_type = StoreType::ER;
    else {
      cerr << "Invalid store type given" << endl;
      exit(1);
    }
  }


  vector<MPIConfigNode> config;
  if ( !config_yaml["Moduli"] )
    config.emplace_back(config_yaml.as<MPIConfigNode>());
  else
    for ( const auto & node : config_yaml["Moduli"] )
      config.emplace_back(node.as<MPIConfigNode>());

  for ( const auto & node : config )
    if ( !node.verify() ) {
      cerr << "Incorrect configuration node:" << endl << node;
      exit(1);
    }


  MPIWorkerPool worker_pool(mpi_world, store_type);

  for ( const auto & node : config ) {
    worker_pool.set_config(node);

    FqElementTable enumeration_table(node.prime, node.prime_exponent);
    CurveIterator iter(enumeration_table, node.genus, node.package_size);
    for (; !iter.is_end(); iter.step() )
      worker_pool.assign(iter.as_block());
  }

  return 0;
}
