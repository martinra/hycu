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
#include <mpi/serialization_tuple.hh>


namespace mpi = boost::mpi;
using boost::optional;
using namespace std;


MPIWorkerPool::
~MPIWorkerPool()
{
  this->wait_for_assigned_blocks();
}

void
MPIWorkerPool::
broadcast_config(
    const MPIConfigNode & config
    )
{
  this->master_thread_pool.update_config(config);
  for ( size_t ix=1; ix<this->mpi_world->size(); ++ix )
    this->mpi_world->send(ix, MPIWorkerPool::update_config_tag, config);
}

void
MPIWorkerPool::
assign(
    vuu_block block
    )
{
  if ( this->opencl_idle.empty() )
    this->fill_idle_queues();

  u_process_id process_id;
  if ( !this->opencl_idle_queue.empty() ) {
    process_id = this->opencl_idle_queue.pop_front();
    if ( process_id == MPIWorkerPool::master_process_id )
      this->master_thread_pool.assign(block);
    else
      this->mpi_world.send(process_id, MPIWorker::assign_opencl_block_tag, block);
  }
  else  {
    process_id = this->cpu_idle_queue.pop_front();
    if ( process_id == MPIWorkerPool::master_process_id )
      this->master_thread_pool.assign(block);
    else
      this->mpi_world.send(process_id, MPIWorker::assign_cpu_block_tag, block);
  }

  this->assigned_blocks[process_id].insert(block);
}

void
MPIWorkerPool::
fill_idle_queues()
{
  tuple<unsigned int, unsigned int> nmb_cpu_opencl;

  while ( true ) {
    nmb_cpu_opencl = this->master_thread_pool.flush_ready_threads();
    for ( size_t jx=0; jx<get<0>(nmb_cpu_opencl); ++jx)
      this->cpu_idle_queue.push_back(MPIWorkerPool::master_process_id)
    for ( size_t jx=0; jx<get<1>(nmb_cpu_opencl); ++jx)
      this->opencl_idle_queue.push_back(MPIWorkerPool::master_process_id)

    for ( size_t ix=1; ix<this->mpi_world->size(); ++ix ) {
      this->mpi_world->send(ix, MPIWorkerPool::flush_ready_threads_tag, true);
      this->mpi_world->recv(ix, MPIWorkerPool::flush_ready_threads_tag, &nmb_cpu_opencl);

      for ( size_t jx=0; jx<get<0>(nmb_cpu_opencl); ++jx)
        this->cpu_idle_queue.push_back(ix)
      for ( size_t jx=0; jx<get<1>(nmb_cpu_opencl); ++jx)
        this->opencl_idle_queue.push_back(ix)
    }

    if ( this->cpu_idle_queue.empty() && this->opencl_idle_queue.empty() )
      this_thread::sleep_for(std::chrono::milliseconds(50));
    else
      break;
  }
}

void
MPIWorkerPool::
flush_finished_blocks()
{
  auto blocks = this->thread_pool->flush_finished_blocks();
  for ( const auto & block : blocks )
    this->finished_block(MPIWorkerPool::master_process_id, block);

  for ( size_t ix=1; ix<this->mpi_world->size(); ++ix ) {
    this->mpi_world.send(ix, MPIWorkerPool::finsihed_blocks_tag, true);
    this->mpi_world.recv(ix, MPIWorkerPool::finsihed_blocks_tag, &blocks);
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
    cerr << "MPIWorkerPool::finish_block: block was not assigned" << endl;
    throw;
  }
  this->assigned_blocks[process_id].erase(block_it);
}

void
MPIWorkerPool::
wait_for_assigned_blocks()
{
  while ( true ) {
    this->flush_finished_blocks()

    for ( const auto & assigned_block : assigned_blocks )
      if ( !assigned_blocks.second().empty() ) {
        this_thread::sleep_for(std::chrono::milliseconds(500));
        continue;
      }
    break;
  }
}
