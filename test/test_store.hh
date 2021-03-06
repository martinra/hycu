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


#ifndef _H_TEST_STORE
#define _H_TEST_STORE

#include <mutex>

#include "store/curve_data.hh"
#include "store/store.hh"
#include "store/store_data.hh"


using std::mutex;
using std::unique_lock;


template<unsigned int prime_power, unsigned int genus, class CurveData, class StoreData>
class TestStore :
  public Store<CurveData, StoreData>
{
  public:
    TestStore() {};

    TestStore(map<typename CurveData::ValueType, typename StoreData::ValueType> store)
    {
      this->store = store;
    };

    inline
    static
    TestStore from_static_store()
    {
      TestStore store;
      store.store = TestStore<prime_power, genus, CurveData, StoreData>::static_store;
      return store;
    };

    void flush_to_static_store(const vuu_block & block) final
    {
      unique_lock<mutex> static_store_lock(TestStore<prime_power, genus, CurveData, StoreData>::static_mutex);

      for ( const auto & item : this->store ) {
        auto store_it = TestStore<prime_power, genus, CurveData, StoreData>::static_store.find(item.first);

        if ( store_it == TestStore<prime_power, genus, CurveData, StoreData>::static_store.end() )
          TestStore<prime_power, genus, CurveData,StoreData>::static_store[item.first] = item.second;
        else
          TestStore<prime_power, genus, CurveData,StoreData>::static_store[item.first] += item.second;
      }
      this->store.clear();
    };

    inline
    bool
    operator!=(
        const TestStore & rhs
        )
    const
    {
      return this->store != rhs.store;
    };

  private:
    static mutex static_mutex;

    static map<typename CurveData::ValueType, typename StoreData::ValueType> static_store;
};

template<unsigned int prime_power, unsigned int genus, class CurveData, class StoreData>
mutex
TestStore<prime_power, genus, CurveData, StoreData>::
static_mutex;

template<unsigned int prime_power, unsigned int genus, class CurveData, class StoreData>
map<typename CurveData::ValueType, typename StoreData::ValueType>
TestStore<prime_power, genus, CurveData, StoreData>::
static_store;


template<unsigned int prime_power, unsigned int genus, class CurveData, class StoreData>
  TestStore<prime_power, genus, CurveData, StoreData> create_reference_store();

#endif
