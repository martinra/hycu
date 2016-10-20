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

#include <boost/filesystem.hpp>
#include <fstream>
#include <sstream>
#include <string>

#include "store/curve_data.hh"
#include "store/file_store.hh"
#include "store/store.hh"
#include "store/store_data.hh"


using namespace std;
using boost::filesystem::is_regular_file;
using boost::filesystem::path;


template<
  class CurveData,
  class StoreData
  >
void
Store<CurveData, StoreData>::
register_curve(
    const Curve & curve
    )
{
  CurveData key_data(curve);
  auto key = key_data.as_value();
  auto twisted_key = key_data.twist().as_value();
  StoreData data(curve);

  auto store_it = this->store.find(key);
  if ( store_it == this->store.end() ) {
    store[key] = data;

    if ( twisted_key == key )
      store[key] += data;
    else
      store[twisted_key] = data.twist();
  }
  else {
    store_it->second += data;
    store[twisted_key] += data;
  }
}

template<
  class CurveData,
  class StoreData
  >
void
Store<CurveData, StoreData>::
flush_to_global_store(
    const vuu_block & block
)
{
  unique_lock<mutex> static_lock(this->static_mutex);

  this->static_record.insert(block);

  for ( auto const& item : this->store ) {
    if ( this->static_store.count(item.first) )
      this->static_store[item.first] += item.second;
    else
      this->static_store[item.first] = item.second;
  }
  this->store.clear();
}

template<
  class CurveData,
  class StoreData
  >
tuple<string, string>
Store<CurveData, StoreData>::
flush_global_store()
{
  unique_lock<mutex> static_lock(this->static_mutex);

  stringstream record_ss;
  FileStore::insert(record_ss, this->static_record);
  this->static_record.clear();

  stringstream store_ss;
  this->insert(store_ss, this->static_store);
  this->static_store.clear();

  return make_tuple(record_ss.str(), store_ss.str());
}

template<
  class CurveData,
  class StoreData
  >
void
Store<CurveData, StoreData>::
extract(
    istream & stream,
    map<typename CurveData::ValueType, typename StoreData::ValueType> & store
    )
{
  char delimiter;

  string line, curve_str, store_str;

  stream.peek();
  while ( !stream.eof() ) {
    getline(stream, line);
    stringstream line_stream(line);

    getline(line_stream, curve_str, ':');
    getline(line_stream, store_str);

    typename CurveData::ValueType curve_value(curve_str);
    typename StoreData::ValueType store_value(store_str);

    if ( store.count(curve_value) )
      store[curve_value] += store_value;
    else
      store[curve_value] = store_value;
  }
}

template<
  class CurveData,
  class StoreData
  >
void
Store<CurveData, StoreData>::
insert(
    ostream & stream,
    const map<typename CurveData::ValueType, typename StoreData::ValueType> & store
    )
{
  for ( auto & store_it : store )
    stream << store_it.first << ":" << store_it.second << endl;
}


template class Store<HyCu::CurveData::ExplicitRamificationHasseWeil, HyCu::StoreData::Count>;
