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

#include "threaded/thread_pool.hh"
#include "opencl_interface.hh"


using namespace std;


void
MPIThreadPool::
spark_threads()
{
  if ( !this->threads.empty() ) return;

  for ( const auto & device : OpenCLInterface::devices() )
    this->threads.push_back(
        make_shared<MPIThread>( shared_from_this(),
                                make_shared<OpenCLInterface>(device) ));

  // each GPU thread accounts for about 1/8 core
  // we slightly oversubscribe here, assuming that the
  // package size in BlockIterator is large enough
  auto nmb_threads = thread::hardware_concurrency();
  for ( size_t ix=this->threads.size()/8; ix<nmb_threads; ++ix )
    this->threads.push_back(make_shared<MPIThread>(shared_from_this()));


  for ( const auto thread : this->threads ) {
    thread->spark();
    this->ready_threads.push_back(thread);
  }
}

void
MPIThreadPool::
shutdown_threads()
{
  for ( auto thread : this->threads )
    thread->shutdown();
}

void
MPIThreadPool::
update_config(
    const MPIConfigNode & config
    )
{
  for ( auto & thread : this->threads )
    thread->update_config(config);
}

void
MPIThreadPool::
assign(
    vuu_block block,
    bool opencl
    )
{
  shared_ptr<MPIThread> thread;
  if ( opencl ) {
    thread = this->idle_threads.front();
    this->idle_threads.pop_front();
  }
  else {
    thread = this->idle_threads.back();
    this->idle_threads.pop_back();
  }

  if ( opencl != thread->is_opencl_thread() ) {
    cerr << "MPIThreadPool::assign: could not meet opencl requirement" << endl;
    throw;
  }

  this->busy_threads[block] = thread;
  thread->assign(block);
}

void
MPIThreadPool::
finished_block(
    vuu_block block
    )
{
  lock_guard<mutex> guard(this->data_mutex);
  
  auto block_it = this->busy_threads.find(block);
  if ( block_it == this->busy_threads.end() ) {
    cerr << "MPIThreadPool::finished_block: block not found" << endl;
    throw; 
  }

  const auto thread = block_it->second;
  this->busy_threads.erase(block_it);
  this->ready_threads.push_back(thread);
  this->finished_blocks.push_back(block);
}

tuple<unsigned int, unsigned int>
MPIThreadPool::
flush_ready_threads()
{
  unsigned int nmb_cpu_threads = 0;
  unsigned int nmb_opencl_threads = 0;

  this->data_mutex.lock();
  for ( auto thread : this->ready_threads ) {
    if ( thread->is_opencl_thread() ) {
      ++nmb_opencl_threads;
      this->idle_threads.push_front(thread);
    }

    else {
      ++nmb_cpu_threads;
      this->idle_threads.push_back(thread);
    }
  }

  this->ready_threads.clear();
  this->data_mutex.unlock();

  return make_tuple(nmb_cpu_threads, nmb_opencl_threads);
}

vector<vuu_block>
MPIThreadPool::
flush_finished_blocks()
{
  lock_guard<mutex>(this->data_mutex);

  return move(finished_blocks);
}