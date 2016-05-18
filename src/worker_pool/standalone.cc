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


#include <vector>
#include <tuple>

#include "worker_pool/standalone.hh"


using namespace std;


StandaloneWorkerPool::
StandaloneWorkerPool(
    shared_ptr<StoreFactoryInterface> store_factory,
    unsigned int nmb_working_threads
    )
{
  this->master_thread_pool = make_shared<ThreadPool>(store_factory);
  this->master_thread_pool->spark_threads(nmb_working_threads);
}

StandaloneWorkerPool::
~StandaloneWorkerPool()
{
  this->wait_for_assigned_blocks();

  this->master_thread_pool->shutdown_threads();
}

void
StandaloneWorkerPool::
set_config(
    const MPIConfigNode & config
    )
{
  this->master_thread_pool->update_config(config);
}

void
StandaloneWorkerPool::
assign(
    vuu_block block
    )
{
  if ( this->nmb_opencl_idle == 0 )
    this->fill_idle_queues();

  if ( this->nmb_opencl_idle != 0 ) {
    --this->nmb_opencl_idle;
    this->master_thread_pool->assign(block, true);
  }
  else {
    --this->nmb_cpu_idle;
    this->master_thread_pool->assign(block, false);
  }

  this->assigned_blocks.insert(block);
}

void
StandaloneWorkerPool::
fill_idle_queues()
{
  tuple<unsigned int, unsigned int> nmb_cpu_opencl;

  while ( true ) {
    this->flush_finished_blocks();

    nmb_cpu_opencl = this->master_thread_pool->flush_ready_threads();
    this->nmb_cpu_idle += get<0>(nmb_cpu_opencl);
    this->nmb_opencl_idle += get<1>(nmb_cpu_opencl);

    if ( this->nmb_cpu_idle == 0 && this->nmb_opencl_idle == 0 )
      this_thread::sleep_for(chrono::milliseconds(50));
    else
      break;
  }
}

void
StandaloneWorkerPool::
flush_finished_blocks()
{
  auto blocks = this->master_thread_pool->flush_finished_blocks();
  for ( const auto & block : blocks )
    this->finished_block(block);
}

void
StandaloneWorkerPool::
finished_block(
    const vuu_block & block
    )
{
  auto block_it = this->assigned_blocks.find(block);
  if ( block_it == this->assigned_blocks.end() ) {
    cerr << "StandaloneWorkerPool::finish_block: block was not assigned:";
    for ( auto bds : block )
      cerr << get<0>(bds) << "," << get<1>(bds) << "; ";
    cerr << endl;
    throw;
  }

  this->assigned_blocks.erase(block_it);
}

void
StandaloneWorkerPool::
wait_for_assigned_blocks()
{
  while ( true ) {
    this->flush_finished_blocks();

    if ( this->assigned_blocks.empty() )
      break;
    else
      this_thread::sleep_for(std::chrono::milliseconds(500));
  }
}
