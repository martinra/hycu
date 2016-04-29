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
#include <map>
#include <memory>
#include <vector>
#include <set>
#include <tuple>

#include <mpi/thread_pool.hh>


namespace mpi = boost::mpi;
using std::deque;
using std::map;
using std::vector;
using std::set;
using std::shared_ptr;
using std::tuple;


typedef unsigned int u_process_id;


class MPIWorkerPool
{
  public:
    MPIWorkerPool(shared_ptr<mpi::communicator> mpi_world);
    ~MPIWorkerPool();

    void broadcast_config(const MPIConfigNode & node);

    void assign(vuu_block);
    void fill_idle_queues();
    void flush_finished_blocks();
    void finished_block(u_process_id process_id, const vuu_block & block);
    void wait_for_assigned_blocks();

    static constexpr unsigned int update_config_tag       = 0;
    static constexpr unsigned int flush_ready_threads_tag = 1;
    static constexpr unsigned int assign_opencl_block_tag = 2;
    static constexpr unsigned int assign_cpu_block_tag    = 3;
    static constexpr unsigned int finished_blocks_tag     = 4;
    static constexpr unsigned int shutdown_tag            = 5;

    static constexpr unsigned int master_process_id = 0;

  private:
    shared_ptr<mpi::communicator> mpi_world;

    shared_ptr<MPIThreadPool> master_thread_pool;

    deque<u_process_id> cpu_idle_queue;
    deque<u_process_id> opencl_idle_queue;

    map<u_process_id, set<vuu_block>> assigned_blocks;
};

#endif
