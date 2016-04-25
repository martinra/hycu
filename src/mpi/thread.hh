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


using std::mutex;


class MPIThread
{
  MPIThread(shared_ptr<ThreadPool> thread_pool);
  MPIThread(shared_ptr<ThreadPool> thread_pool, const cl::Device & device);

  bool inline has_opencl() const { return (bool)this->opencl; };

  void main();

  void update_config(const MPIConfigNode & config);
  void assign( vuu_block block,
               shared_ptr<FqElementTable> fq_table,
               vector<shared_ptr<ReductionTable>> reduction_tables );

  private:
    shared_ptr<ThreadPool> thread_pool;

    thread thread;
    mutex main_mutex;
    mutex data_mutex;

    MPIConfigNode config;

    shared_ptr<OpenCLInterface> opencl;
    shared_ptr<FqElementTable> fq_table;
    vector<shared_ptr<ReductionTable>> reduction_tables;

    deque< tuple<vuu_block, shared_ptr<FqElementTable>, vector<shared_ptr<ReductionTable>> > blocks;


    void check_blocks();
};

#endif
