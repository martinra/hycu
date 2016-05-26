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

#include "threaded/thread.hh"
#include "threaded/thread_pool.hh"


using namespace std;


void
Thread::
spark()
{
  this->shutting_down = false;
  this->main_std_thread = thread( Thread::main_thread, shared_from_this(), this->store_factory );
}

void
Thread::
shutdown()
{
  this->data_mutex.lock();
  this->shutting_down = true;
  this->data_mutex.unlock();

  this->main_cond_var.notify_all();
  this->main_std_thread.join();
}

void
Thread::
main_thread(
    shared_ptr<Thread> thread,
    const shared_ptr<StoreFactoryInterface> store_factory
    )
{
  unique_lock<mutex> main_lock(thread->main_mutex);

  while ( true ) {
    while ( true ) {
      thread->data_mutex.lock();
      bool shutting_down = thread->shutting_down;
      bool blocks_empty = thread->blocks.empty();
      thread->data_mutex.unlock();

      if ( shutting_down )
        return;

      if ( blocks_empty )
        thread->main_cond_var.wait(main_lock);
      else
        break;
    }


    vuu_block block;
    shared_ptr<FqElementTable> fq_table;
    vector<shared_ptr<ReductionTable>> reduction_tables;
    shared_ptr<ConfigNode> config;

    thread->data_mutex.lock();
    tie(block, config, fq_table, reduction_tables) = thread->blocks.front();
    thread->blocks.pop_front();
    thread->data_mutex.unlock();


    auto store = store_factory->create();
    for ( BlockIterator iter(block); !iter.is_end(); iter.step() ) {
      Curve curve(fq_table, iter.as_position());
      if ( !curve.has_squarefree_rhs() ) continue;
      for ( auto table : reduction_tables ) curve.count(table);
      store->register_curve(curve);
    }


    store->save(*config, block);

    auto thread_pool_shared = thread->thread_pool.lock();
    if ( thread_pool_shared )
      thread_pool_shared->finished_block(block);
    else {
      cerr << "Thread::main_thread: expired thread_pool in thread "
           << this_thread::get_id() << endl;
      throw;
    }
    thread_pool_shared.reset();
  }
}

void
Thread::
update_config(
    const ConfigNode & config
    )
{
  this->config = make_shared<ConfigNode>(config);

  this->fq_table = make_shared<FqElementTable>(config.prime, config.prime_exponent);
  this->reduction_tables.clear();
  for ( size_t fx=config.genus; fx>config.genus/2; --fx )
    this->reduction_tables.push_back(make_shared<ReductionTable>(config.prime, fx, this->opencl));
}

void
Thread::
assign(
    vuu_block block
    )
{
  this->data_mutex.lock();
  this->blocks.emplace_back(block, this->config, this->fq_table, this->reduction_tables);
  this->data_mutex.unlock();

  this->main_cond_var.notify_one();
}
