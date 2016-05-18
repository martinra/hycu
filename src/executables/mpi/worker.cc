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


#include "executables/mpi/worker.hh"
#include "store/store_factory.hh"
#include "threaded/thread_pool.hh"
#include "utils/serialization_tuple.hh"
#include "worker_pool/mpi.hh"


namespace mpi = boost::mpi;
using namespace std;


int
main_worker(
    shared_ptr<mpi::communicator> mpi_world
    )
{
  StoreType store_type;
  unsigned int nmb_working_threads;
  MPIWorkerPool::broadcast_initialization(mpi_world, store_type, nmb_working_threads);

  auto thread_pool = make_shared<ThreadPool>(create_store_factory(store_type));
  thread_pool->spark_threads(nmb_working_threads);


  while ( true ) {
    mpi::status mpi_status = mpi_world->probe();

    if ( mpi_status.tag() == MPIWorkerPoolTag::update_config ) {
      MPIConfigNode config;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::update_config, config );
      thread_pool->update_config(config);
    }

    else if ( mpi_status.tag() == MPIWorkerPoolTag::assign_opencl_block ) {
      vuu_block block;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::assign_opencl_block, block );
      thread_pool->assign(block, true);
    }

    else if ( mpi_status.tag() == MPIWorkerPoolTag::assign_cpu_block ) {
      vuu_block block;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::assign_cpu_block, block );
      thread_pool->assign(block, false);
    }

    else if ( mpi_status.tag() == MPIWorkerPoolTag::flush_ready_threads ) { 
      bool dummy;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::flush_ready_threads, dummy );

      mpi_world->send( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::flush_ready_threads,
                       thread_pool->flush_ready_threads() );
    }

    else if ( mpi_status.tag() == MPIWorkerPoolTag::finished_blocks ) {
      bool dummy;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::finished_blocks, dummy );
      mpi_world->send( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::finished_blocks,
                       thread_pool->flush_finished_blocks() );
    }

    else if ( mpi_status.tag() == MPIWorkerPoolTag::shutdown ) {
      bool dummy;
      mpi_world->recv( MPIWorkerPool::master_process_id,
                       MPIWorkerPoolTag::shutdown, dummy );
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
