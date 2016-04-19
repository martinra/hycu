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


#include <boost/mpi.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <yaml-cpp/yaml.h>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>
#include <block_iterator.hh>
#include <curve_iterator.hh>
#include <isogeny_representative_store.hh>
#include <mpi/config_node.hh>
#include <mpi/worker_pool.hh>
#include <mpi/serialization_tuple.hh>


namespace mpi = boost::mpi;
using namespace std;


int main_master(int argc, char** argv, mpi::communicator & mpi_world);
int main_worker(mpi::communicator & mpi_world);
string representative_output_name(
    string result_folder, unsigned int prime_power, const vector<tuple<int,int>> & bounds);

int
main(
    int argc,
    char** argv
    )
{
  mpi::environment mpi_environment(argc, argv);
  mpi::communicator mpi_world;

  if (mpi_world.rank() == 0)
    return main_master(argc, argv, mpi_world);
  else
    return main_worker(mpi_world);
}

int
main_master(
    int argc,
    char** argv,
    mpi::communicator & mpi_world
    )
{
  if (argc != 2) {
    cerr << "One argument, the configuration file, is needed" << endl;
    exit(1);
  }

  auto config_yaml = YAML::LoadFile(argv[1]);

  auto config = config_yaml.as<MPIConfigNode>();
  if ( !config.verify() ) {
    cerr << "Incorrect configuration file:" << endl << config;
    exit(1);
  }

  auto mpi_worker_pool = MPIWorkerPool(mpi_world);
  mpi::broadcast(mpi_world, config, 0);


  // todo: receive opencl capabilities and hosts
  //       group by hosts
  //       decide for each host group which process runs which accelarators
  //       transmit this information
  // OpenCL directives
  mpi_world.send(1, 0, true);
  for ( size_t rank=2; rank<mpi_world.size(); ++rank )
    mpi_world.send(rank, 0,false);


  FqElementTable enumeration_table(config.prime, config.prime_exponent);

  for ( auto curve_enumerator = CurveIterator(enumeration_table, config.genus, config.package_size);
        !curve_enumerator.is_end();
        curve_enumerator.step() )
    // todo: check whether results are there
    mpi_worker_pool.emit( curve_enumerator.as_block() );


  mpi_worker_pool.close_pool();

  return 0;
}

int
main_worker(
    mpi::communicator & mpi_world
    )
{
  MPIConfigNode config;

  mpi::broadcast(mpi_world, config, 0);

  int prime_power = pow(config.prime, config.prime_exponent);


  // OpenCL directives
  bool use_opencl;
  mpi_world.recv(0, 0, use_opencl);


  auto fq_table = make_shared<FqElementTable>(config.prime, config.prime_exponent);
  auto opencl = use_opencl ? make_shared<OpenCLInterface>() : shared_ptr<OpenCLInterface>();

  vector<ReductionTable> reduction_tables;
  for ( size_t fx=config.genus; fx>config.genus/2; --fx )
    reduction_tables.emplace_back(config.prime, fx, opencl);


  while (true) {
    mpi_world.send(0, 0, true);

    mpi::status mpi_status = mpi_world.probe();
    if (mpi_status.tag() == 1) {
      bool close;
      mpi_world.recv(mpi_status.source(), 1, close);
      if (close) return 0;
    } else if (mpi_status.source() != 0 || mpi_status.tag() != 0) {
      cerr << "worker" << mpi_world.rank() << " unexpected tag " << mpi_status.tag() << endl;
      exit(1);
    }

    // use a skeleton here
    vector<tuple<int,int>> bounds;
    mpi_world.recv(0, 0, bounds);
    
    IsogenyRepresentativeStore isogeny_representative_store;
    for ( BlockIterator iter(bounds); !iter.is_end(); iter.step() ) {
      Curve curve(fq_table, iter.as_position());
      for ( auto & table : reduction_tables ) curve.count(table);
      isogeny_representative_store.register_curve(curve);

     // todo: use this to sign off computation, and assert that process was not killed
     // mpi::send(0, 1, bounds);
    }

    fstream( representative_output_name(config.result_path.native(), prime_power, bounds),
             ios_base::out )
      << isogeny_representative_store;
  }
}

string
representative_output_name(
    string result_path,
    unsigned int prime_power,
    const vector<tuple<int,int>> & bounds
    )
{
  stringstream output_name(ios_base::out);

  output_name << result_path << "/isogeny_representatives";

  output_name << "__prime_power_" << prime_power;
  output_name << "__coeff_exponent_bounds";
  for ( auto bds : bounds )
    output_name << "__" << get<0>(bds) << "_" << get<1>(bds);

  output_name << ".hycu_unmerged";

  return output_name.str();
}
