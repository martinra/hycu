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


#ifndef _H_MPI_WORKER_POOL
#define _H_MPI_WORKER_POOL

#include <boost/mpi.hpp>
#include <deque>
#include <vector>
#include <set>
#include <tuple>

using std::deque;
using std::vector;
using std::set;
using std::tuple;


class MPIWorkerPool
{
  public:
    MPIWorkerPool(boost::mpi::communicator & mpi_world) :
      mpi_world( mpi_world ) {};

    void emit(vector<tuple<int,int>> &&);
    void wait_for_all_working();
    void wait_until_one_more_idle();
    void close_pool();

  private:
    const boost::mpi::communicator & mpi_world;

    set<int> working_processes;
    deque<int> idle_processes;
};

#endif

