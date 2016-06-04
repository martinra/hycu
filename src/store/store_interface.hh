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


#ifndef _H_STORE_STORE_INTERFACE
#define _H_STORE_STORE_INTERFACE


#include "block_iterator.hh"
#include "config/config_node.hh"
#include "curve.hh"


using std::istream;
using std::ostream;


class StoreInterface
{
  public:
    virtual bool was_inserted(const Curve & curve) const = 0;
    virtual void insert(const Curve & curve) = 0;
    virtual bool was_saved(const ConfigNode & config, const vuu_block & block) const = 0;
    virtual void save(const ConfigNode & config, const vuu_block & block) = 0;

    friend ostream & operator<<(ostream & stream, const StoreInterface & store);
    friend istream & operator>>(istream & stream, StoreInterface & store);

  private:
    virtual ostream & insert_store(ostream & stream) const = 0;
    virtual istream & extract_store(istream & stream) = 0; 
};

inline
ostream &
operator<<(
    ostream & stream,
    const StoreInterface & store
    )
{
  return store.insert_store(stream);
};

inline
istream &
operator>>(
    istream & stream,
    StoreInterface & store
    )
{
  return store.extract_store(stream);
};

#endif