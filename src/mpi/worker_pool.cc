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


#include <boost/mpi/communicator.hpp>
#include <vector>
#include <tuple>

#include <mpi/worker_pool.hh>
#include <mpi/serialization_tuple.hh>


namespace mpi = boost::mpi;
using namespace std;


void
MPIWorkerPool::
emit(
    vector<tuple<int,int>> && coeff_bounds
    )
{
  if ( this->idle_processes.empty() ) {
    this->wait_until_one_more_idle();
  }

  int rank = this->idle_processes.front();
  this->idle_processes.pop_front();
  this->working_processes.insert(rank);

  this->mpi_world.send(rank, 0, vector<tuple<int,int>>(coeff_bounds));
}

void
MPIWorkerPool::
wait_for_all_working()
{
  while ( !this->working_processes.empty() )
    this->wait_until_one_more_idle();
}

void
MPIWorkerPool::
wait_until_one_more_idle()
{
  mpi::status mpi_status = this->mpi_world.probe();

  if ( mpi_status.tag() != 0 )
    throw;

  bool idle_status;
  this->mpi_world.recv(mpi_status.source(), 0, idle_status);

  if ( idle_status ) {
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
  for ( int rank : this->idle_processes )
    this->mpi_world.send(rank, 1, true);
}
