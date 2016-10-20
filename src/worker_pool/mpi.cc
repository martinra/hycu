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
#include <chrono>
#include <vector>
#include <thread>
#include <tuple>

#include "store/file_store.hh"
#include "utils/serialization_tuple.hh"
#include "worker_pool/mpi.hh"


namespace mpi = boost::mpi;
using namespace std;


constexpr
unsigned int
MPIWorkerPool::
master_process_id;


MPIWorkerPool::
MPIWorkerPool(
    shared_ptr<mpi::communicator> mpi_world,
    StoreType store_type,
    int nmb_working_threads,
    unsigned int nmb_threads_per_gpu
    ) :
  mpi_world ( mpi_world ),
  next_save_time ( system_clock::now() + chrono::minutes(5) )
{
  MPIWorkerPool::broadcast_initialization( mpi_world,
      store_type, nmb_working_threads, nmb_threads_per_gpu );

  auto store_factory = create_store_factory(store_type);
  this->master_thread_pool = make_shared<ThreadPool>(store_factory);
  this->master_thread_pool->spark_threads(nmb_working_threads, nmb_threads_per_gpu);
}

MPIWorkerPool::
~MPIWorkerPool()
{
  this->wait_for_assigned_blocks();
  this->save_global_stores_to_file();

  {
    unique_lock<mutex> mpi_lock(this->mpi_mutex);
    for ( u_process_id ix = 1; ix < this->mpi_world->size(); ++ix )
      mpi_world->send(ix, MPIWorkerPoolTag::shutdown, true);
  }
  this->master_thread_pool->shutdown_threads();
}

void
MPIWorkerPool::
broadcast_initialization(
    shared_ptr<mpi::communicator> mpi_world,
    StoreType & store_type,
    int & nmb_working_threads,
    unsigned int & nmb_threads_per_gpu
    )
{
  // mpi_mutex not aquired, since this is a static method
  mpi::broadcast(*mpi_world, store_type, MPIWorkerPool::master_process_id);
  mpi::broadcast(*mpi_world, nmb_working_threads, MPIWorkerPool::master_process_id);
  mpi::broadcast(*mpi_world, nmb_threads_per_gpu, MPIWorkerPool::master_process_id);
}

void
MPIWorkerPool::
update_config(
    const ConfigNode & config
    )
{
  this->wait_for_assigned_blocks();
  this->save_global_stores_to_file();
  
  this->file_store = make_shared<FileStore>(config);
  this->master_thread_pool->update_config(config);

  unique_lock<mutex> mpi_lock(this->mpi_mutex);
  for ( size_t ix=1; ix<this->mpi_world->size(); ++ix )
    this->mpi_world->send(ix, MPIWorkerPoolTag::update_config, config);
}

void
MPIWorkerPool::
assign(
    vuu_block block
    )
{
  this->delayed_save_global_stores_to_file();

  if ( this->file_store->contains(block) )
    return;


  if ( this->opencl_idle_queue.empty() )
    this->fill_idle_queues();

  u_process_id process_id;
  if ( !this->opencl_idle_queue.empty() ) {
    process_id = this->opencl_idle_queue.front();
    this->opencl_idle_queue.pop_front();

    if ( process_id == MPIWorkerPool::master_process_id )
      this->master_thread_pool->assign(block, true);
    else {
      unique_lock<mutex> mpi_lock(this->mpi_mutex);
      this->mpi_world->send(process_id, MPIWorkerPoolTag::assign_opencl_block, block);
    }
  }
  else  {
    process_id = this->cpu_idle_queue.front();
    this->cpu_idle_queue.pop_front();

    if ( process_id == MPIWorkerPool::master_process_id )
      this->master_thread_pool->assign(block, false);
    else {
      unique_lock<mutex> mpi_lock(this->mpi_mutex);
      this->mpi_world->send(process_id, MPIWorkerPoolTag::assign_cpu_block, block);
    }
  }

  this->assigned_blocks[process_id].insert(block);
}

void
MPIWorkerPool::
fill_idle_queues()
{
  tuple<unsigned int, unsigned int> nmb_cpu_opencl;

  while ( true ) {
    this->flush_finished_blocks();

    nmb_cpu_opencl = this->master_thread_pool->flush_ready_threads();
    for ( size_t jx=0; jx<get<0>(nmb_cpu_opencl); ++jx)
      this->cpu_idle_queue.push_back(MPIWorkerPool::master_process_id);
    for ( size_t jx=0; jx<get<1>(nmb_cpu_opencl); ++jx)
      this->opencl_idle_queue.push_back(MPIWorkerPool::master_process_id);

    {
      unique_lock<mutex> mpi_lock(this->mpi_mutex);
      for ( size_t ix=1; ix<this->mpi_world->size(); ++ix ) {
        this->mpi_world->send(ix, MPIWorkerPoolTag::flush_ready_threads, true);
        this->mpi_world->recv(ix, MPIWorkerPoolTag::flush_ready_threads, nmb_cpu_opencl);

        for ( size_t jx=0; jx<get<0>(nmb_cpu_opencl); ++jx)
          this->cpu_idle_queue.push_back(ix);
        for ( size_t jx=0; jx<get<1>(nmb_cpu_opencl); ++jx)
          this->opencl_idle_queue.push_back(ix);
      }
    }

    if ( this->cpu_idle_queue.empty() && this->opencl_idle_queue.empty() )
      this_thread::sleep_for(chrono::milliseconds(50));
    else
      break;
  }
}

void
MPIWorkerPool::
flush_finished_blocks()
{
  auto blocks = this->master_thread_pool->flush_finished_blocks();
  for ( const auto & block : blocks )
    this->finished_block(MPIWorkerPool::master_process_id, block);

  unique_lock<mutex> mpi_lock(this->mpi_mutex);
  for ( u_process_id ix=1; ix<this->mpi_world->size(); ++ix ) {
    blocks.clear();
    this->mpi_world->send(ix, MPIWorkerPoolTag::finished_blocks, true);
    this->mpi_world->recv(ix, MPIWorkerPoolTag::finished_blocks, blocks);
    for ( const auto & block : blocks )
      this->finished_block(ix, block);
  }
}

void
MPIWorkerPool::
finished_block(
    u_process_id process_id,
    const vuu_block & block
    )
{
  auto block_it = this->assigned_blocks[process_id].find(block);
  if ( block_it == this->assigned_blocks[process_id].end() ) {
    cerr << "MPIWorkerPool::finish_block: block was not assigned to process " << process_id << ": ";
    for ( auto bds : block )
      cerr << get<0>(bds) << "," << get<1>(bds) << "; ";
    cerr << endl;
    throw;
  }

  this->assigned_blocks[process_id].erase(block_it);
}

void
MPIWorkerPool::
wait_for_assigned_blocks()
{
  bool remaining_blocks = true;
  while ( remaining_blocks ) {
    this->flush_finished_blocks();

    remaining_blocks = false;
    for ( auto assigned_block : assigned_blocks )
      if ( !assigned_block.second.empty() ) {
        remaining_blocks = true;
        this_thread::sleep_for(std::chrono::milliseconds(500));
        break;
      }
  }
}

void
MPIWorkerPool::
save_global_stores_to_file()
{
  this->file_store->save(master_thread_pool->flush_global_store());

  for ( size_t ix=1; ix<this->mpi_world->size(); ++ix ) {
    tuple<string, string> record_store;
    {
      unique_lock<mutex> mpi_lock(this->mpi_mutex);
      this->mpi_world->send(ix, MPIWorkerPoolTag::save_global_stores_to_file, true);
      this->mpi_world->recv(ix, MPIWorkerPoolTag::save_global_stores_to_file, record_store);
    }
    this->file_store->save(record_store);
  }
}
