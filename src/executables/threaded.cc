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


#include <iostream>
#include <vector>
#include <tuple>
#include <yaml-cpp/yaml.h>

#include "curve_iterator.hh"
#include "config/config_node.hh"
#include "store/store_factory.hh"
#include "threaded/thread_pool.hh"


using namespace std;


int
main(
    int argc,
    char** argv
    )
{
  if (argc != 2) {
    cerr << "One argument, the configuration file, is needed" << endl;
    exit(1);
  }

  auto config_yaml = YAML::LoadFile(argv[1]);


  StoreType store_type;
  if ( !config_yaml["StoreType"] )
    store_type = StoreType::EC;
  else {
    auto store_type_str = config_yaml["StoreType"].as<string>();
    if ( store_type_str == "EC" )
      store_type = StoreType::EC;
    else if ( store_type_str == "ER" )
      store_type = StoreType::ER;
    else {
      cerr << "Invalid store type given" << endl;
      exit(1);
    }
  }


  vector<MPIConfigNode> config;
  if ( !config_yaml["Moduli"] )
    config.emplace_back(config_yaml.as<MPIConfigNode>());
  else
    for ( const auto & node : config_yaml["Moduli"] )
      config.emplace_back(node.as<MPIConfigNode>());

  for ( const auto & node : config )
    if ( !node.verify() ) {
      cerr << "Incorrect configuration node:" << endl << node;
      exit(1);
    }


  auto thread_pool = make_shared<MPIThreadPool>(create_store_factory(store_type));
  thread_pool->spark_threads();

  unsigned int nmb_idle_cpu = 0;
  unsigned int nmb_idle_opencl = 0;
  set<vuu_block> assigned_blocks;

  for ( const auto & node : config ) {
    thread_pool->update_config(node);

    FqElementTable enumeration_table(node.prime, node.prime_exponent);
    CurveIterator iter(enumeration_table, node.genus, node.package_size);
    for (; !iter.is_end(); iter.step() ) {
      auto block_ptr = iter.as_block();
      while ( true ) {
        if ( nmb_idle_opencl > 0 ) {
          thread_pool->assign(block_ptr, true);
          --nmb_idle_opencl;
          assigned_blocks.insert(block_ptr);
          break;
        }
        else if ( nmb_idle_cpu > 0 ) {
          thread_pool->assign(block_ptr, false);
          --nmb_idle_cpu;
          assigned_blocks.insert(block_ptr);
          break;
        }
        else {
          auto ready_threads = thread_pool->flush_ready_threads();
          nmb_idle_cpu += get<0>(ready_threads);
          nmb_idle_opencl += get<1>(ready_threads);

          auto finished_blocks = thread_pool->flush_finished_blocks();
          for ( const auto & block : finished_blocks ) {
            if ( assigned_blocks.find(block) == assigned_blocks.end() ) {
              cerr << "finished block was not assigned: ";
              for ( auto pt : block )
                cerr << get<0>(pt) << "," << get<1>(pt) << "; ";
              cerr << endl;
              exit(1);
            }
            assigned_blocks.erase(block_ptr);
          }
          this_thread::sleep_for(chrono::milliseconds(50));
        }
      }
    }
  }
  thread_pool->shutdown_threads();


  return 0;
}
