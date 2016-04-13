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
#include <yaml.h>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>
#include <block_iterator.hh>
#include <curve_iterator.hh>
#include <isogeny_representative_store.hh>
#include <mpi_worker_pool.hh>


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

  auto config = YAML::LoadFile(argv[1]);

  if (   !config["prime"] || !config["primeExponent"]
      || !config["genus"] || !config["resultFolder"] ) {
    cerr << "configuration file must give prime, prime exponent, maximal prime exponent, "
         << "genus, and result folder" << endl;
    exit(1);
  }

  int prime = config["prime"].as<int>();
  int prime_exponent = config["primeExponent"].as<int>();
  int genus = config["genus"].as<int>();
  int package_size = config["packageSize"].as<int>();
  string result_folder = config["resultFolder"].as<string>();
  // todo: test that config variables are valid
  int degree = 2*genus + 2;

  auto mpi_worker_pool = MPIWorkerPool(mpi_world);
  mpi::broadcast(mpi_world, prime, 0);
  mpi::broadcast(mpi_world, prime_exponent, 0);
  mpi::broadcast(mpi_world, genus, 0);
  mpi::broadcast(mpi_world, result_folder, 0);

  FqElementTable enumeration_table(prime, prime_exponent);

  for ( auto curve_enumerator = CurveIterator(enumeration_table, genus, package_size);
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
  int prime;
  int prime_exponent;
  int genus;
  string result_folder;

  mpi::broadcast(mpi_world, prime, 0);
  mpi::broadcast(mpi_world, prime_exponent, 0);
  mpi::broadcast(mpi_world, genus, 0);
  mpi::broadcast(mpi_world, result_folder, 0);

  int prime_power = pow(prime, prime_exponent);


  auto fq_table = make_shared<FqElementTable>(prime, prime_exponent);
  auto opencl = make_shared<OpenCLInterface>();

  vector<ReductionTable> reduction_tables;
  for ( size_t fx=genus; fx>genus/2; --fx )
    reduction_tables.emplace_back(prime, fx, opencl);


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

    fstream( representative_output_name(result_folder, prime_power, bounds),
             ios_base::out )
      << isogeny_representative_store;
  }
}

string
representative_output_name(
    string result_folder,
    unsigned int prime_power,
    const vector<tuple<int,int>> & bounds
    )
{
  stringstream output_name(ios_base::out);

  output_name << result_folder << "/isogeny_representatives";

  output_name << "__prime_power_" << prime_power;
  output_name << "__coeff_exponent_bounds";
  for ( auto bds : bounds )
    output_name << "__" << get<0>(bds) << "_" << get<1>(bds);

  output_name << ".data";

  return output_name.str();
}
