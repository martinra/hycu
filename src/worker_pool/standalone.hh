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
#include "store/store_factory.hh"


using std::set;
using std::shared_ptr;


class StandaloneWorkerPool
{
  public:
    StandaloneWorkerPool(StoreType store_type);
    ~StandaloneWorkerPool();

    void set_config(const MPIConfigNode & node);

    void assign(vuu_block);
    void fill_idle_queues();
    void flush_finished_blocks();
    void finished_block(const vuu_block & block);
    void wait_for_assigned_blocks();

  private:
    shared_ptr<ThreadPool> master_thread_pool;

    unsigned int nmb_cpu_idle;
    unsigned int nmb_opencl_idle;

    set<vuu_block> assigned_blocks;
};

#endif
