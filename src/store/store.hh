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


#ifndef _H_STORE_STORE
#define _H_STORE_STORE

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "block_iterator.hh"
#include "config/config_node.hh"
#include "curve.hh"


using std::istream;
using std::map;
using std::mutex;
using std::ostream;
using std::set;
using std::string;
using std::vector;


class StoreInterface
{
  public:
    virtual void register_curve(const Curve & curve) = 0;
    virtual void flush_to_static_store(const vuu_block & block) = 0;
    virtual tuple<string, string> flush_static_store() = 0;

    virtual void extract(istream & stream) = 0;
    virtual void extract(istream && stream) = 0;
    virtual void insert(ostream & stream) const = 0;
    virtual void insert(ostream && stream) const = 0;
};


template<class CurveData, class StoreData>
class Store :
  public StoreInterface
{
  public:
    void register_curve(const Curve & curve) final;
    void flush_to_static_store(const vuu_block & block);
    tuple<string, string> flush_static_store();

    void
    extract(
        istream & stream
        )
    final
    {
      this->extract(stream, this->store);
    };

    inline
    void
    extract(
        istream && stream
        )
    final
    {
      this->extract(stream);
    };

    void
    insert(
        ostream & stream
        )
    const final
    {
      this->insert(stream, this->store);
    };

    inline
    void
    insert(
        ostream && stream
        )
    const final
    {
      this->insert(stream);
    };

  protected:

    // derived test_store has to access this
    map<typename CurveData::ValueType, typename StoreData::ValueType> store;

  private:

    static void extract(
        istream & stream,
        map<typename CurveData::ValueType, typename StoreData::ValueType> & store
        );

    static void insert(
        ostream & stream,
        const map<typename CurveData::ValueType, typename StoreData::ValueType> & store
        );



    static mutex static_mutex;
    static map<typename CurveData::ValueType, typename StoreData::ValueType> static_store;
    static set<vuu_block> static_record;
};


template<class CurveData, class StoreData>
mutex
Store<CurveData, StoreData>::static_mutex;

template<class CurveData, class StoreData>
map<typename CurveData::ValueType, typename StoreData::ValueType>
Store<CurveData, StoreData>::static_store;

template<class CurveData, class StoreData>
set<vuu_block>
Store<CurveData, StoreData>::static_record;

#endif
