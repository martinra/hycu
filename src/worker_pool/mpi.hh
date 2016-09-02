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


#ifndef _H_WORKER_POOL_MPI
#define _H_WORKER_POOL_MPI

#include <boost/mpi.hpp>
#include <deque>
#include <map>
#include <memory>
#include <set>

#include "threaded/thread_pool.hh"
#include "store/store.hh"
#include "store/store_factory.hh"


namespace mpi = boost::mpi;
using std::deque;
using std::map;
using std::set;
using std::shared_ptr;


typedef unsigned int u_process_id;


class MPIWorkerPool
{
  public:
    MPIWorkerPool(
        shared_ptr<mpi::communicator> mpi_world,
        StoreType store_type,
        int nmb_working_threads = -1,
        unsigned int nmb_threads_per_gpu = 0
        );
    ~MPIWorkerPool();

   static void
   broadcast_initialization(
       shared_ptr<mpi::communicator> mpi_world,
       StoreType & store_type,
       int & nmb_working_threads,
       unsigned int & nmb_threads_per_gpu
       );

    void set_config(const ConfigNode & node);

    void assign(vuu_block);
    void fill_idle_queues();
    void flush_finished_blocks();
    void finished_block(u_process_id process_id, const vuu_block & block);
    void wait_for_assigned_blocks();

    static constexpr unsigned int master_process_id = 0;

  private:
    shared_ptr<ThreadPool> master_thread_pool;

    shared_ptr<mpi::communicator> mpi_world;

    deque<u_process_id> cpu_idle_queue;
    deque<u_process_id> opencl_idle_queue;

    shared_ptr<StoreInterface> store;
    ConfigNode store_config;

    map<u_process_id, set<vuu_block>> assigned_blocks;
};


enum MPIWorkerPoolTag
{
  store_type,
  update_config,
  flush_ready_threads,
  assign_opencl_block,
  assign_cpu_block,
  finished_blocks,
  shutdown
};

#endif
