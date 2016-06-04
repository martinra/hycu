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

#include "store/store_isomorphism_class.hh"
#include "store/store_factory.hh"


using namespace std;
using boost::filesystem::is_regular_file;
using boost::filesystem::path;



void
StoreHasseWeilIsomorphismClass::
insert(
    const Curve & curve
    )
{
  HasseWeil key_data(curve);
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
    store[twisted_key] += data.twist();
  }
}

template<
  class CurveData,
  class StoreData
  >
bool
Store<CurveData, StoreData>::
was_saved(
    const ConfigNode & config,
    const vuu_block & block
    ) const
{
  return is_regular_file( path(this->output_file_name(config, block)) );
}

template<
  class CurveData,
  class StoreData
  >
void
Store<CurveData, StoreData>::
save(
    const ConfigNode & config,
    const vuu_block & block
)
{
  fstream(this->output_file_name(config, block), ios_base::out) << *this;
}

template<
  class CurveData,
  class StoreData
  >
string
Store<CurveData, StoreData>::
output_file_name(
    const ConfigNode & config,
    const vuu_block & block
    ) const
{
  stringstream output_name(ios_base::out);
  output_name << "store";

  output_name << "__prime_power_" << pow(config.prime, config.prime_exponent);
  output_name << "__coeff_exponent_bounds";
  for ( auto bds : block )
    output_name << "__" << get<0>(bds) << "_" << get<1>(bds);

  output_name << ".hycu_unmerged";

  return (config.result_path / path(output_name.str())).native();
}

template<
  class CurveData,
  class StoreData
  >
ostream &
Store<CurveData, StoreData>::
insert_store(
    ostream & stream
    )
  const
{
  for ( auto & store_it : this->store )
    stream << store_it.first << ":" << store_it.second << endl;

  return stream;
}

template<
  class CurveData,
  class StoreData
  >
istream &
Store<CurveData, StoreData>::
extract_store(
    istream & stream
    )
{
  char delimiter;

  typename CurveData::ValueType curve_value;
  typename StoreData::ValueType store_value;

  stream.peek();
  while ( !stream.eof() ) {
    stream >> curve_value;
    delimiter = stream.peek();
    if ( delimiter != ':' ) {
      cerr << "operator>> Store: cannot extract: character after curve value is " << delimiter << endl;
      throw;
    }
    stream.ignore(1);

    stream >> store_value;
    delimiter = stream.peek();
    if ( delimiter != '\n' ) {
      cerr << "operator>> Store: cannot extract: character after store value is " << delimiter <<  endl;
      throw;
    }
    stream.ignore(1);

    auto store_it = this->store.find(curve_value);
    if ( store_it == this->store.end() )
      this->store[curve_value] = store_value;
    else
      this->store[curve_value] += store_value;

    stream.peek();
  }

  return stream;
}
