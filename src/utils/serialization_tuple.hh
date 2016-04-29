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


#ifndef _H_MPI_SERIALIZATION_TUPLE
#define _H_MPI_SERIALIZATION_TUPLE

#include <boost/serialization/serialization.hpp>
#include <tuple>

using std::get;

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
