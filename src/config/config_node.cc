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


#include <ostream>

#include "config/config_node.hh"


using namespace std;
using namespace YAML;


ostream &
operator<<(
    ostream & stream,
    const MPIConfigNode & config
    )
{
  stream << "base field: " << config.prime << "^" << config.prime_exponent << "; ";
  stream << "genus: " << config.genus << "; ";
  stream << "result_path: " << config.result_path.generic_string() << "; ";
  stream << "package_size: " << config.package_size << endl;

  return stream;
}


namespace YAML
{

  Node
  convert<MPIConfigNode>::
  encode(
      const MPIConfigNode & config
      )
  {
    Node node;
  
    node["Prime"] = config.prime;
    node["PrimeExponent"] = config.prime_exponent;
    node["Genus"] = config.genus;
  
    node["ResultPath"] = config.result_path.generic_string();
  
    node["PackageSize"] = config.package_size;
  
    return node;
  }
  
  bool
  convert<MPIConfigNode>::
  decode(
      const Node & node,
      MPIConfigNode & config
      )
  {
    if (    !node["Prime"] || !node["PrimeExponent"]
         || !node["Genus"]
         || !node["ResultPath"]
         || !node["PackageSize"] )
      return false;
  
    config.prime = node["Prime"].as<int>();
    config.prime_exponent = node["PrimeExponent"].as<int>();
  
    config.genus = node["Genus"].as<int>();
  
    config.result_path = path(node["ResultPath"].as<string>());
  
    config.package_size = node["PackageSize"].as<int>();
  
    return true;
  }

}
