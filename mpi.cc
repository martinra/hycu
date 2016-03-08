#include <boost/mpi.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <yaml.h>

#include <count.hh>
#include <mpi_worker_pool.hh>

namespace mpi = boost::mpi;
using namespace std;


int main_master(int argc, char** argv, mpi::communicator & mpi_world);
int main_worker(mpi::communicator & mpi_world);

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

  if (!config["prime"] || !config["genus"] || !config["resultFolder"]) {
    cerr << "configuration file must give prime, genus, and result folder" << endl;
    exit(1);
  }

  int prime = config["prime"].as<int>();
  int genus = config["genus"].as<int>();
  int package_size = config["packageSize"].as<int>();
  string result_folder = config["resultFolder"].as<string>();
  // todo: test that config variables are valid
  int degree = 2*genus + 2;

  auto mpi_worker_pool = MPIWorkerPool(mpi_world);
  mpi::broadcast(mpi_world, prime, 0);
  mpi::broadcast(mpi_world, genus, 0);
  mpi::broadcast(mpi_world, result_folder, 0);


  for ( auto curve_enumerator = CurveEnumerator(prime, genus, package_size);
        !curve_enumerator.is_end();
        curve_enumerator.step() )
    // todo: check whether results are there
    mpi_worker_pool.emit( curve_enumerator.as_bounds() );


  mpi_worker_pool.close_pool();

  return 0;
}

int
main_worker(
    mpi::communicator & mpi_world
    )
{
  int prime;
  int genus;
  string result_folder;

  mpi::broadcast(mpi_world, prime, 0);
  mpi::broadcast(mpi_world, genus, 0);
  mpi::broadcast(mpi_world, result_folder, 0);
  int degree = 2*genus + 2;


  auto opencl = OpenCLInterface();

  vector<ReductionTableFq> tables;
  for (int px = 0; px < genus; ++px)
    tables.emplace_back(ReductionTableFq(prime,px, opencl));


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
    vector<tuple<int,int>> coeff_bounds;
    mpi_world.recv(0, 0, coeff_bounds);

    stringstream output_name(ios_base::out);
    output_name << result_folder << "/coeff_bounds";
    for (auto bds : coeff_bounds)
      output_name << "__" << get<0>(bds) << "_" << get<1>(bds);
    output_name << ".curve_count";
    fstream output(output_name.str(), ios_base::out);

    CurveCounter(prime, coeff_bounds)
      .count( [&output](auto poly_coeffs, auto nmb_points)
              {
                 for (auto c : poly_coeffs) output << c << " ";
                 output << ": ";
                 for (auto pts : nmb_points) output << get<0>(pts) << " " << get<1>(pts) << " ";
                 output << endl;
              },
              tables, opencl );

    // use this to sign off computation, and assert that process was not killed
    // mpi::send(0, 1, coeff_bounds);
  }
}
