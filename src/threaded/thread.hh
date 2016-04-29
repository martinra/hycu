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


#ifndef _H_MPI_THREAD
#define _H_MPI_THREAD

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <thread>
#include <tuple>

#include "block_iterator.hh"
#include "fq_element_table.hh"
#include "config/config_node.hh"
#include "opencl_interface.hh"
#include "reduction_table.hh"


using std::condition_variable;
using std::deque;
using std::mutex;
using std::shared_ptr;
using std::thread;
using std::tuple;
using std::weak_ptr;


class MPIThreadPool;


class MPIThread :
  public std::enable_shared_from_this<MPIThread>
{
  public:
    MPIThread( weak_ptr<MPIThreadPool> thread_pool,
               shared_ptr<OpenCLInterface> opencl = {} ) :
      thread_pool ( thread_pool ), opencl ( opencl ) {};
  
    void spark();
    void shutdown();

    bool inline is_opencl_thread() const { return (bool)this->opencl; };
  

    template<class Store>
      static void main_thread(shared_ptr<MPIThread> thread);
  
    void update_config(const MPIConfigNode & config);
    void assign(vuu_block block);

  private:
    weak_ptr<MPIThreadPool> thread_pool;

    thread main_std_thread;
    bool shutting_down;

    mutex main_mutex;
    mutex data_mutex;
    condition_variable main_cond_var;

    MPIConfigNode config;
    
    shared_ptr<OpenCLInterface> opencl;
    shared_ptr<FqElementTable> fq_table;
    vector<shared_ptr<ReductionTable>> reduction_tables;

    deque<tuple< vuu_block, shared_ptr<FqElementTable>, vector<shared_ptr<ReductionTable>> >> blocks;
};

#endif
