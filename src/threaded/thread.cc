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

#include <fstream>

#include "store/count_representative.hh"
#include "threaded/thread.hh"
#include "threaded/thread_pool.hh"


using namespace std;


void
MPIThread::
spark()
{
  this->shutting_down = false;
  this->main_std_thread = thread( MPIThread::main_thread<StoreCountRepresentative>,
                                  shared_from_this() );
}

void
MPIThread::
shutdown()
{
  this->shutting_down = true;
  this->main_cond_var.notify_all();
  this->main_std_thread.join();
}

template<class Store>
void
MPIThread::
main_thread(
    shared_ptr<MPIThread> thread
    )
{
  unique_lock<mutex> main_lock(thread->main_mutex);
  unique_lock<mutex> data_lock(thread->data_mutex, defer_lock);

  while ( true ) {
    while ( true ) {
      if ( thread->shutting_down )
        return;

      if ( thread->blocks.empty() )
        thread->main_cond_var.wait(main_lock);
      else
        break;
    }


    vuu_block block;
    shared_ptr<FqElementTable> fq_table;
    vector<shared_ptr<ReductionTable>> reduction_tables;

    data_lock.lock();
    tie(block, fq_table, reduction_tables) = thread->blocks.front();
    thread->blocks.pop_front();
    data_lock.unlock();


    Store store;
    for ( BlockIterator iter(block); !iter.is_end(); iter.step() ) {
      Curve curve(fq_table, iter.as_position());
      if ( !curve.has_squarefree_rhs() ) continue;
      for ( auto table : reduction_tables ) curve.count(table);
      store.register_curve(curve);
    }


    data_lock.lock();
    fstream(store.output_file_name(thread->config, block), ios_base::out) << store;

    auto thread_pool_shared = thread->thread_pool.lock();
    if ( thread_pool_shared )
      thread_pool_shared->finished_block(block);
    else {
      cerr << "MPIThread::main_thread: expired thread_pool in thread "
           << this_thread::get_id() << endl;
      throw;
    }
    thread_pool_shared.reset();
    data_lock.unlock();
  }
}

void
MPIThread::
update_config(
    const MPIConfigNode & config
    )
{
  this->config = config;

  this->fq_table = make_shared<FqElementTable>(config.prime, config.prime_exponent);
  this->reduction_tables.clear();
  for ( size_t fx=config.genus; fx>config.genus/2; --fx )
    this->reduction_tables.push_back(make_shared<ReductionTable>(config.prime, fx, this->opencl));
}

void
MPIThread::
assign(
    vuu_block block
    )
{
  this->data_mutex.lock();
  this->blocks.emplace_back(block, this->fq_table, this->reduction_tables);
  this->data_mutex.unlock();

  this->main_cond_var.notify_one();
}
