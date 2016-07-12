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

#include <string>
#include <vector>

#include "block_iterator.hh"
#include "config/config_node.hh"
#include "curve.hh"


using std::istream;
using std::ostream;
using std::string;
using std::vector;


class StoreInterface
{
  public:
    virtual void register_curve(const Curve & curve) = 0;
    virtual bool was_saved(const ConfigNode & config, const vuu_block & block) = 0;
    virtual void save(const ConfigNode & config, const vuu_block & block) = 0;

    friend
    inline
    ostream &
    operator<<(
        ostream & stream,
        const StoreInterface & store
        )
    {
      return store.insert(stream);
    };

    virtual void extract(istream & stream) = 0;

  private:
    virtual ostream & insert(ostream & stream) const = 0;
};


template<class CurveData, class StoreData> class Store;

template<class CurveData, class StoreData>
ostream & operator<<(ostream & stream, const Store<CurveData,StoreData> & store);


template<class CurveData, class StoreData>
class Store :
  public StoreInterface
{
  public:
    void register_curve(const Curve & curve) final;
    bool was_saved(const ConfigNode & config, const vuu_block & block);
    void save(const ConfigNode & config, const vuu_block & block);

    friend ostream & operator<< <> (ostream & stream, const Store<CurveData,StoreData> & store);
    void extract(istream & stream) final;

  protected:
    map<typename CurveData::ValueType, typename StoreData::ValueType> store;

  private:
    ostream & insert(ostream & stream) const final;


    string output_file_name(const ConfigNode & config, const vuu_block & block);
};

template<class CurveData, class StoreData>
inline
ostream &
operator<<(
    ostream & stream,
    const Store<CurveData,StoreData> & store
    )
{
  return store.insert(stream);
};

#endif
