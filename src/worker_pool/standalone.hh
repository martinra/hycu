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


#ifndef _H_WORKER_POOL_STANDALONE
#define _H_WORKER_POOL_STANDALONE

#include <memory>
#include <set>

#include "threaded/thread_pool.hh"
#include "store/file_store.hh"


using std::chrono::system_clock;
using std::set;
using std::shared_ptr;


class StandaloneWorkerPool
{
  public:
    StandaloneWorkerPool(
        shared_ptr<StoreFactoryInterface> store_factory,
        int nmb_working_threads = -1,
        unsigned int nmb_threads_per_gpu = 0
        );

    StandaloneWorkerPool(
        StoreType store_type,
        int nmb_working_threads = -1,
        unsigned int nmb_threads_per_gpu = 0
        ) :
      StandaloneWorkerPool ( create_store_factory(store_type), nmb_working_threads, nmb_threads_per_gpu ) {};

    ~StandaloneWorkerPool();


    void assign(vuu_block);
    void fill_idle_queues();
    void finished_block(const vuu_block & block);
    void flush_finished_blocks();
    void save_global_stores_to_file();
    void update_config(const ConfigNode & node);
    void wait_for_assigned_blocks();

    inline
    void
    delayed_save_global_stores_to_file()
    {
      if ( system_clock::now() > this->next_save_time ) {
        this->save_global_stores_to_file();
      }
    };

  private:
    shared_ptr<ThreadPool> master_thread_pool;

    unsigned int nmb_cpu_idle = 0;
    unsigned int nmb_opencl_idle = 0;
    set<vuu_block> assigned_blocks;

    shared_ptr<FileStore> file_store;
    system_clock::time_point next_save_time;
    static const std::chrono::minutes delay_save_time;
};

#endif
