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
#include <boost/optional.hpp>
#include <vector>
#include <tuple>

#include <mpi/worker_pool.hh>
#include <utils/serialization_tuple.hh>


namespace mpi = boost::mpi;
using boost::optional;
using namespace std;


constexpr
unsigned int
MPIWorkerPool::
update_config_tag;

constexpr
unsigned int
MPIWorkerPool::
flush_ready_threads_tag;

constexpr
unsigned int
MPIWorkerPool::
assign_opencl_block_tag;

constexpr
unsigned int
MPIWorkerPool::
assign_cpu_block_tag;

constexpr
unsigned int
MPIWorkerPool::
finished_blocks_tag;

constexpr
unsigned int
MPIWorkerPool::
shutdown_tag;

constexpr
unsigned int
MPIWorkerPool::
master_process_id;


MPIWorkerPool::
MPIWorkerPool(
    shared_ptr<mpi::communicator> mpi_world
    ) :
  mpi_world ( mpi_world )
{
  cerr << "mpi_world references in work pool ctor: " << mpi_world.use_count() << endl;

  this->master_thread_pool = make_shared<MPIThreadPool>();
  this->master_thread_pool->spark_threads();
}

MPIWorkerPool::
~MPIWorkerPool()
{
  cerr << "work pool dtor" << endl;
  this->wait_for_assigned_blocks();
  cerr << "waited for assigned blocks" << endl;

  for ( u_process_id ix = 1; ix < this->mpi_world->size(); ++ix )
      mpi_world->send(ix, MPIWorkerPool::shutdown_tag, true);
  cerr << "sent all shutdown messages" << endl;
  this->master_thread_pool->shutdown_threads();

  cerr << "mpi_world references in work pool dtor: " << mpi_world.use_count() << endl;
  cerr << "work pool destructor run in " << this_thread::get_id() << endl;
}

void
MPIWorkerPool::
broadcast_config(
    const MPIConfigNode & config
    )
{
  this->master_thread_pool->update_config(config);
  for ( size_t ix=1; ix<this->mpi_world->size(); ++ix )
    this->mpi_world->send(ix, MPIWorkerPool::update_config_tag, config);
}

void
MPIWorkerPool::
assign(
    vuu_block block
    )
{
  if ( this->opencl_idle_queue.empty() )
    this->fill_idle_queues();

  u_process_id process_id;
  if ( !this->opencl_idle_queue.empty() ) {
    process_id = this->opencl_idle_queue.front();
    this->opencl_idle_queue.pop_front();

    if ( process_id == MPIWorkerPool::master_process_id )
      this->master_thread_pool->assign(block, true);
    else
      this->mpi_world->send(process_id, MPIWorkerPool::assign_opencl_block_tag, block);
  }
  else  {
    process_id = this->cpu_idle_queue.front();
    this->cpu_idle_queue.pop_front();

    if ( process_id == MPIWorkerPool::master_process_id )
      this->master_thread_pool->assign(block, false);
    else
      this->mpi_world->send(process_id, MPIWorkerPool::assign_cpu_block_tag, block);
  }

//  cerr << "assigning to process " << process_id << ": ";
//  for ( auto bds : block )
//    cerr << get<0>(bds) << "," << get<1>(bds) << "; ";
//  cerr << endl;
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

    for ( size_t ix=1; ix<this->mpi_world->size(); ++ix ) {
      this->mpi_world->send(ix, MPIWorkerPool::flush_ready_threads_tag, true);
      this->mpi_world->recv(ix, MPIWorkerPool::flush_ready_threads_tag, nmb_cpu_opencl);

      for ( size_t jx=0; jx<get<0>(nmb_cpu_opencl); ++jx)
        this->cpu_idle_queue.push_back(ix);
      for ( size_t jx=0; jx<get<1>(nmb_cpu_opencl); ++jx)
        this->opencl_idle_queue.push_back(ix);
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
  for ( const auto & block : blocks ) {
//    cerr << "finished block: ";
//    for ( auto bds : block )
//      cerr << get<0>(bds) << "," << get<1>(bds) << "; ";
//    cerr << endl;

    this->finished_block(MPIWorkerPool::master_process_id, block);
  }

  blocks.clear();
  for ( u_process_id ix=1; ix<this->mpi_world->size(); ++ix ) {
    this->mpi_world->send(ix, MPIWorkerPool::finished_blocks_tag, true);
    this->mpi_world->recv(ix, MPIWorkerPool::finished_blocks_tag, blocks);
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
//  else {
//    cerr << "MPIWorkerPool::finish_block: block was assigned to process " << process_id << ": ";
//    for ( auto bds : block )
//      cerr << get<0>(bds) << "," << get<1>(bds) << "; ";
//    cerr << endl;
//  }
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
