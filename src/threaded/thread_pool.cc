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
#include "opencl/interface.hh"


using namespace std;


void
ThreadPool::
spark_threads(
    unsigned int nmb_working_threads,
    unsigned int nmb_threads_per_gpu
    )
{
  if ( !this->threads.empty() ) {
    cerr << "ThreadPool::spark_threads: threads already run" << endl;
    throw;
  }


  if ( nmb_working_threads == 0 )
    nmb_working_threads = thread::hardware_concurrency();

#ifdef WITH_OPENCL
  for ( const auto & device : OpenCLInterface::devices() )
    for ( unsigned int ix = 0; ix < nmb_threads_per_gpu; ++ix )
      this->threads.push_back(
          make_shared<Thread>( shared_from_this(), this->store_factory,
                               make_shared<OpenCLInterface>(device) ));
#endif

  // each GPU thread accounts for about 1/8 core
  // we slightly oversubscribe here, assuming that the
  // package size in BlockIterator is large enough
  for ( size_t ix=this->threads.size()/8; ix<nmb_working_threads; ++ix )
    this->threads.push_back(make_shared<Thread>(shared_from_this(), this->store_factory));


  for ( const auto thread : this->threads ) {
    thread->spark();
    this->ready_threads.push_back(thread);
  }
}

void
ThreadPool::
shutdown_threads()
{
  for ( auto thread : this->threads )
    thread->shutdown();
  this->threads.clear();
}

void
ThreadPool::
update_config(
    const ConfigNode & config
    )
{
  for ( auto & thread : this->threads )
    thread->update_config(config);
}

void
ThreadPool::
assign(
    vuu_block block,
    bool opencl
    )
{
  unique_lock<mutex> data_lock(this->data_mutex);

  shared_ptr<Thread> thread;
  if ( opencl ) {
    thread = this->idle_threads.front();
    this->idle_threads.pop_front();
  }
  else {
    thread = this->idle_threads.back();
    this->idle_threads.pop_back();
  }

  if ( opencl != thread->is_opencl_thread() ) {
    cerr << "ThreadPool::assign: could not meet opencl requirement" << endl;
    throw;
  }

  this->busy_threads[block] = thread;
  thread->assign(block);
}

void
ThreadPool::
finished_block(
    vuu_block block
    )
{
  unique_lock<mutex> data_lock(this->data_mutex);
  
  auto block_it = this->busy_threads.find(block);
  if ( block_it == this->busy_threads.end() ) {
    cerr << "ThreadPool::finished_block: block not found" << endl;
    throw; 
  }

  this->ready_threads.push_back(block_it->second);
  this->finished_blocks.push_back(block);
  this->busy_threads.erase(block_it);
}

tuple<unsigned int, unsigned int>
ThreadPool::
flush_ready_threads()
{
  unique_lock<mutex> data_lock(this->data_mutex);

  unsigned int nmb_cpu_threads = 0;
  unsigned int nmb_opencl_threads = 0;

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

  return make_tuple(nmb_cpu_threads, nmb_opencl_threads);
}

vector<vuu_block>
ThreadPool::
flush_finished_blocks()
{
  unique_lock<mutex> data_lock(this->data_mutex);

  return move(finished_blocks);
}
