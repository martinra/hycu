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

#include <mpi/thread.hh>


MPIThread::
MPIThread(
    shared_ptr<ThreadPool> thread_pool
    ) :
  thread_pool ( thread_pool )
{

  this->main_mutex.lock();
  this->thread = thread(MPIThread::main());
}

MPIThread::
MPIThread(
    shared_ptr<ThreadPool> thread_pool,
    const cl::Device & device
    ) :
  thread_pool ( thread_pool )
{
  this->opencl = make_shared<OpenCLInterface>(device);

  this->main_mutex.lock();
  this->thread = thread(this->main);
}

void
MPIThread::
main()
{
  while (true) {
    this->main_mutex.lock();

    vuu_block block;
    shared_ptr<FqElementTable> fq_table;
    vector<shared_ptr<ReductionTable>> reduction_tables;

    this->data_mutex.lock();
    tie(block, fq_table, reduction_tables) = this->blocks->pop_front();
    this->data_mutex.unlock();

    MPIStore store(this->config, block);
    for ( BlockIterator iter(block); !iter.is_end(); iter.step() ) {
      Curve curve(fq_table, iter.as_position());
      for ( auto table : reduction_tables ) curve.count(*table);
      store.register_curve(curve);
    }

    store.write_to_file();
    thread_pool->finished_block(block);

    this->check_blocks();
  }
}

void
update_config(
    const MPIConfigNode & config
    )
{
  this->config = config;

  this->fq_table = make_shared<FqElementTable>(config.prime, config.prime_exponent);
  this->reduction_tables.clear();
  for ( size_t fx=config.genus; fx>config.genus/2; --fx )
    this->reduction_tables.emplace_back(config.prime, fx, this->opencl);
}

void
MPIThread::
assign(
    vuu_block block,
    )
{
  this->data_mutex.lock();
  this->block.emplace_back(make_tupe(block, this->fq_table, this->reduction_tables));
  this->main_mutex.unlock();
  this->data_mutex.unlock();
}

void
MPIThread::
check_blocks()
{
  this->data_mutex.lock();
  if ( !this->blocks.empty() )
    this->main_mutex.unlock();
  this->data_mutex.unlock();
}
