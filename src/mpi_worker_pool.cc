#include <boost/mpi/communicator.hpp>
#include <vector>
#include <tuple>

#include <mpi_worker_pool.hh>

void
MPIWorkerPool::
emit(
    vector<tuple<int,int>> && coeff_bounds
    )
{
  if (this->idle_processes.size() == 0)
    this->wait_until_one_more_idle();

  int rank = this->idle_processes.front();
  this->idle_processes.pop_front();
  this->working_processes.insert(rank);

  this->mpi_world.send(rank, 0, vector<tuple<int,int>>(coeff_bounds));
}

void
MPIWorkerPool::
wait_for_all_working()
{
  while (this->working_processes.size() != 0)
    this->wait_until_one_more_idle();
}

void
MPIWorkerPool::
wait_until_one_more_idle()
{
  mpi::status mpi_status = this->mpi_world.probe();

  if (mpi_status.tag() != 0)
    throw;

  bool idle_status;
  this->mpi_world.recv(mpi_status.source(), 0, idle_status);

  if (idle_status) {
    this->working_processes.erase(mpi_status.source());
    this->idle_processes.push_back(mpi_status.source());
  } else
    this->wait_until_one_more_idle();
}

void
MPIWorkerPool::
close_pool()
{
  wait_for_all_working();
  for (int rank : this->idle_processes)
    this->mpi_world.send(rank, 1, true);
}
