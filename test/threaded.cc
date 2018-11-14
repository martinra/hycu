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


#include <boost/test/unit_test.hpp>

#include "test_store.hh"
#include "worker_pool/standalone.hh"

#include "reference_store_q5_g1.hh"
#include "reference_store_q7_g1.hh"
#include "reference_store_q7_g2.hh"


using namespace std;


BOOST_AUTO_TEST_CASE( threaded_q5_g1 )
{
  auto worker_pool = make_shared<StandaloneWorkerPool>(
         dynamic_pointer_cast<StoreFactoryInterface>( make_shared<
           StoreFactory<TestStore<5,1, HyCu::CurveData::ExplicitRamificationHasseWeil,HyCu::StoreData::Count>>
           >() )
         );

  ConfigNode node;
  node.prime = 5;
  node.prime_exponent = 1;
  node.genus = 1;
  node.count_exponent = 1;
  node.package_size = 30;

  worker_pool->update_config(node);
  FqElementTable enumeration_table(node.prime, node.prime_exponent);
  CurveIterator iter(enumeration_table, node.genus, node.package_size);
  for (; !iter.is_end(); iter.step() )
    worker_pool->assign(iter.as_block());

  worker_pool.reset();
  auto computed_store = TestStore<5,1, HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>::from_static_store();
  auto reference_store = create_reference_store<5,1, HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>();

  if ( computed_store != reference_store ) {
    stringstream message;
    computed_store.insert( message << "threaded computation of genus 1 curves / F_5:" << endl );
     
    BOOST_FAIL( message.str() );
  }
}

BOOST_AUTO_TEST_CASE( threaded_q7_g1 )
{
  auto worker_pool = make_shared<StandaloneWorkerPool>(
         dynamic_pointer_cast<StoreFactoryInterface>( make_shared<
           StoreFactory<TestStore<7,1, HyCu::CurveData::ExplicitRamificationHasseWeil,HyCu::StoreData::Count>>
           >() )
         );

  ConfigNode node;
  node.prime = 7;
  node.prime_exponent = 1;
  node.genus = 1;
  node.count_exponent = 1;
  node.package_size = 30;

  worker_pool->update_config(node);
  FqElementTable enumeration_table(node.prime, node.prime_exponent);
  CurveIterator iter(enumeration_table, node.genus, node.package_size);
  for (; !iter.is_end(); iter.step() )
    worker_pool->assign(iter.as_block());

  worker_pool.reset();
  auto computed_store = TestStore<7,1, HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>::from_static_store();
  auto reference_store = create_reference_store<7,1, HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>();

  if ( computed_store != reference_store ) {
    stringstream message;
    computed_store.insert( message << "threaded computation of genus 1 curves / F_7:" << endl );
     
    BOOST_FAIL( message.str() );
  }
}

BOOST_AUTO_TEST_CASE( threaded_q7_g2 )
{
  auto worker_pool = make_shared<StandaloneWorkerPool>(
         dynamic_pointer_cast<StoreFactoryInterface>( make_shared<
           StoreFactory<TestStore<7,2, HyCu::CurveData::ExplicitRamificationHasseWeil,HyCu::StoreData::Count>>
           >() )
         );

  ConfigNode node;
  node.prime = 7;
  node.prime_exponent = 1;
  node.genus = 2;
  node.count_exponent = 2;
  node.package_size = 30;

  worker_pool->update_config(node);
  FqElementTable enumeration_table(node.prime, node.prime_exponent);
  CurveIterator iter(enumeration_table, node.genus, node.package_size);
  for (; !iter.is_end(); iter.step() )
    worker_pool->assign(iter.as_block());

  worker_pool.reset();
  auto computed_store = TestStore<7,2, HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>::from_static_store();
  auto reference_store = create_reference_store<7,2, HyCu::CurveData::ExplicitRamificationHasseWeil,HyCu::StoreData::Count>();

  if ( computed_store != reference_store ) {
    stringstream message;
    computed_store.insert( message << "threaded computation of genus 2 curves / F_7:" << endl );
     
    BOOST_FAIL( message.str() );
  }
}
