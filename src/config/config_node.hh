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


#ifndef _H_MPI_CONFIG_NODE
#define _H_MPI_CONFIG_NODE

#include <boost/filesystem.hpp>
#include <ostream>
#include <string>
#include <yaml-cpp/yaml.h>


using boost::filesystem::path;
using boost::filesystem::is_directory;
using std::ostream;
using std::string;


struct MPIConfigNode
{
  unsigned int prime;
  unsigned int prime_exponent;
  
  unsigned int genus;
  
  path result_path;
  
  unsigned int package_size;


  inline bool verify() const
  {
    return (    prime != 0 && prime_exponent != 0
             && genus != 0
             && (is_directory(result_path) || create_directories(result_path))
             && package_size != 0
           );
  };

  inline void prepend_output_path(const path & output_path)
  {
    this->result_path = output_path / this->result_path;
  };
};

ostream & operator<<(ostream & stream, const MPIConfigNode & config);


namespace boost {
namespace serialization {

  template <class Archive>
  void
  serialize(
      Archive & ar,
      MPIConfigNode & config,
      const unsigned int version
      )
  {
    ar & config.prime;
    ar & config.prime_exponent;

    ar & config.genus;

    string result_path_str;
    if ( Archive::is_saving::value )
      result_path_str = config.result_path.generic_string();
    ar & result_path_str;
    if ( ! Archive::is_saving::value )
      config.result_path = path(result_path_str);

    ar & config.package_size;
  }

}}


namespace YAML
{

  template<>
  struct convert<MPIConfigNode>
  {
    static Node encode(const MPIConfigNode & config);
    static bool decode(const Node & node, MPIConfigNode & config);
  };

}

#endif
