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


#ifndef _H_STORE_GLOBAL_STORE
#define _H_STORE_GLOBAL_STORE

#include <boost/filesystem.hpp>
#include <sstream>
#include <string>

#include "block_iterator.hh"
#include "config/config_node.hh"

using boost::filesystem::path;
using boost::filesystem::is_regular_file;
using std::string;
using std::stringstream;


class GlobalStore
{
  public:
    GlobalStore(
        const ConfigNode & config
        ) :
      config ( config )
    {};

    inline
    bool
    contains(
        vuu_block & block
        )
    {
      return is_regular_file( path(this->output_file_name(this->config, block)) );
    };

    static
    inline
    string
    output_file_name(
        const ConfigNode & config,
        const vuu_block & block
        )
    {
      stringstream output_name(std::ios_base::out);
      output_name << "store";

      output_name << "__prime_power_" << pow(config.prime, config.prime_exponent);
      output_name << "__coeff_exponent_bounds";
      for ( auto bds : block )
        output_name << "__" << std::get<0>(bds) << "_" << std::get<1>(bds);

      output_name << ".hycu_unmerged";

      return (config.result_path / path(output_name.str())).native();
    };

  private:
    ConfigNode config;
};

#endif
