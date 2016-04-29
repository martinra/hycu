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


#include "mpi/worker.hh"
#include "mpi/worker_pool.hh"
#include "threaded/thread_pool.hh"
#include "utils/serialization_tuple.hh"


namespace mpi = boost::mpi;
using namespace std;


int
main_worker(
    shared_ptr<mpi::communicator> mpi_world
    )
{
  auto thread_pool = make_shared<MPIThreadPool>();
  thread_pool->spark_threads();

  while ( true ) {
    mpi::status mpi_status = mpi_world->probe();

    if ( mpi_status.tag() == MPIWorkerPool::update_config_tag ) {
      MPIConfigNode config;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::update_config_tag, config );
      thread_pool->update_config(config);
    }

    else if ( mpi_status.tag() == MPIWorkerPool::assign_opencl_block_tag ) {
      vuu_block block;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::assign_opencl_block_tag, block );
      thread_pool->assign(block, true);
    }

    else if ( mpi_status.tag() == MPIWorkerPool::assign_cpu_block_tag ) {
      vuu_block block;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::assign_cpu_block_tag, block );
      thread_pool->assign(block, false);
    }

    else if ( mpi_status.tag() == MPIWorkerPool::flush_ready_threads_tag ) { 
      bool dummy;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::flush_ready_threads_tag, dummy );

      mpi_world->send( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::flush_ready_threads_tag,
                       thread_pool->flush_ready_threads() );
    }

    else if ( mpi_status.tag() == MPIWorkerPool::finished_blocks_tag ) {
      bool dummy;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::finished_blocks_tag, dummy );
      mpi_world->send( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::finished_blocks_tag,
                       thread_pool->flush_finished_blocks() );
    }

    else if ( mpi_status.tag() == MPIWorkerPool::shutdown_tag ) {
      bool dummy;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPool::shutdown_tag, dummy );
      thread_pool->shutdown_threads();
      break;
    }

    else {
      cerr << "main_worker: unrecognized MPI message tag" << endl;
      exit(1);
    }
  }

  return 0;
}
