#ifndef _H_MPI_WORKER_POOL
#define _H_MPI_WORKER_POOL

#include <boost/mpi.hpp>
#include <boost/serialization/serialization.hpp>
#include <deque>
#include <vector>
#include <set>
#include <tuple>

namespace mpi = boost::mpi;
using namespace std;


class MPIWorkerPool
{
  public:
    MPIWorkerPool(mpi::communicator & mpi_world) :
      mpi_world( mpi_world ) {};

    void emit(vector<tuple<int,int>> &&);
    void wait_for_all_working();
    void wait_until_one_more_idle();
    void close_pool();

  private:
    const mpi::communicator & mpi_world;

    set<int> working_processes;
    deque<int> idle_processes;
};


namespace boost {
namespace serialization {

  template <unsigned int N>
  struct Serializer
  {
    template<class Archive, typename... Args>
    static void serialize(Archive & ar, std::tuple<Args...> & a, const unsigned int version)
    {
      ar & get<N-1>(a);
      Serializer<N-1>::serialize(ar, a, version);
    }
  };
  
  template<>
  struct Serializer<0>
  {
    template<class Archive, typename... Args>
    static void serialize(Archive & ar, std::tuple<Args...> & a, const unsigned int version) {}
  };

  template <class Archive, typename... Args>
  void serialize(Archive & ar, std::tuple<Args...> & a, const unsigned int version)
  {
    Serializer<sizeof...(Args)>::serialize(ar, a, version);
  }

}}

#endif

