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


#include <mpi/master.hh>


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

  vector<MPIConfigNode> config;
  if ( config_yaml.isSequence() )
    for ( const auto & node : config_yaml )
      config.emplace_back(node.as<MPIConfigNode>());
  else
    config.emplace_back(config_yaml.as<MPIConfigNode>());

  for ( const auto & node : config )
    if ( !config.verify() ) {
      cerr << "Incorrect configuration node:" << endl << config;
      exit(1);
    }


  MPIWorkerPool mpi_worker_pool(mpi_world);

  for ( const auto & node : config ) {
    mpi_worker_pool.broadcast_config(node);

    FqElementTable enumeration_table(node.prime, node.prime_exponent);
    CurveIterator iter(enumeration_table, config.genus, config.package_size);
    for (; !iter.is_end(); iter.step() )
      mpi_worker_pool.assign(curve_enumerator.as_block());
  }


  return 0;
}
