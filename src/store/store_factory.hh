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


#ifndef _H_STORE_STORE_FACTORY
#define _H_STORE_STORE_FACTORY

#include <memory>

#include "store/curve_data.hh"
#include "store.hh"
#include "store/store_data.hh"


using std::shared_ptr;
using std::make_shared;
using std::dynamic_pointer_cast;


class StoreFactoryInterface
{
  public:
    virtual shared_ptr<StoreInterface> create() const = 0;
};


template<class Store>
class StoreFactory :
  public StoreFactoryInterface
{
  inline shared_ptr<StoreInterface> create() const final
  {
    return dynamic_pointer_cast<StoreInterface>(make_shared<Store>());
  };
};


// todo: choose more descriptive names
enum StoreType { EC, ER };

inline
const shared_ptr<StoreFactoryInterface>
create_store_factory(
    StoreType store_type
    )
{
  switch ( store_type ) {
    case StoreType::EC:
      return dynamic_pointer_cast<StoreFactoryInterface>(
          make_shared< StoreFactory<Store<HyCu::CurveData::ExplicitRamificationHasseWeil,
                                          HyCu::StoreData::Count>>
                     >() );
      break;

    case StoreType::ER:
      return dynamic_pointer_cast<StoreFactoryInterface>(
          make_shared< StoreFactory<Store<HyCu::CurveData::ExplicitRamificationHasseWeil,
                                          HyCu::StoreData::Representative>>
                     >() );
      break;

    default:
      throw;
  }
};

#endif