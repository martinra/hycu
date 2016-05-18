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


#ifndef _H_MPI_THREAD_POOL
#define _H_MPI_THREAD_POOL

#include <thread>

#include "block_iterator.hh"
#include "store/store_factory.hh"
#include "threaded/thread.hh"


using std::deque;
using std::map;
using std::mutex;
using std::vector;
using std::set;
using std::shared_ptr;
using std::tuple;


class ThreadPool :
  public std::enable_shared_from_this<ThreadPool>
{
  public:
    ThreadPool(const shared_ptr<StoreFactoryInterface> store_factory) :
      store_factory ( store_factory ) {};

    void spark_threads(unsigned int nmb_working_threads = 0);
    void shutdown_threads();

    void update_config(const MPIConfigNode & config);

    void assign(vuu_block block, bool opencl);
    void finished_block(vuu_block block);

    vector<vuu_block> flush_finished_blocks();
    tuple<unsigned int, unsigned int> flush_ready_threads();

  private:
    const shared_ptr<StoreFactoryInterface> store_factory;

    mutex data_mutex;

    vector<shared_ptr<Thread>> threads;
    deque<shared_ptr<Thread>> idle_threads;
    vector<shared_ptr<Thread>> ready_threads;
    map<vuu_block, shared_ptr<Thread>> busy_threads;

    vector<vuu_block> finished_blocks;
};

#endif

