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


#ifndef _H_STORE_STORE_GLOBAL
#define _H_STORE_STORE_GLOBAL

#include <string>
#include <vector>

#include "block_iterator.hh"
#include "config/config_node.hh"
#include "curve.hh"
#include "store/store.hh"


template<class CurveData, class StoreData>
class StoreGlobal :
  public StoreInterface
{
  public:
    inline
    bool
    was_inserted(
        const Curve & curve
        ) final
    {
      shared_lock store_lock(store_mutex);

      return    CurveData::was_inserted(this->store, curve)
             || StoreData::was_inserted(this->store, curve);
    };
     
    void insert(const Curve & curve) final;

    bool was_saved(const ConfigNode & config, const vuu_block & block) const;
    void save(const ConfigNode & config, const vuu_block & block);

  protected:
    static mutex store_mutex;
    static map< typename CurveData::ValueType,
                HyCu::StoreData::IsomorphismClass::ValueType >
                  store;

  private:
    ostream & insert_store(ostream & stream) const final;
    istream & extract_store(istream & stream) final;

    string output_file_name(const ConfigNode & config, const vuu_block & block);
};

#endif
