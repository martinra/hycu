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


#ifndef _H_STORE_FILE_STORE
#define _H_STORE_FILE_STORE

#include <boost/filesystem.hpp>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "block_iterator.hh"
#include "config/config_node.hh"

using boost::filesystem::path;
using std::istream;
using std::ostream;
using std::set;
using std::string;
using std::stringstream;


class FileStore
{
  public:
    FileStore(const ConfigNode & config);

    inline
    bool
    contains(
        const vuu_block & block
        )
    const
    {
      return this->initial_record.count(block);
    }

    void save( tuple<string, string> record_store );
    tuple<path, path> new_filenames();

    inline
    path
    full_path(string filename)
    const
    {
      return config.result_path / path(filename);
    };

    static istream & extract(istream & stream, vuu_block & block);
    static void extract(istream & stream, set<vuu_block> & record);

    static ostream & insert(ostream & stream, const vuu_block & block);
    static void insert(ostream & stream, const set<vuu_block> & record);

    inline
    static
    istream &
    extract(
        istream && stream,
        vuu_block & block
        )
    {
      return FileStore::extract(stream, block);
    };

    inline
    static
    void
    extract(
        istream && stream,
        set<vuu_block> & record
        )
    {
      return FileStore::extract(stream, record);
    };

    inline
    static
    ostream &
    insert(
        ostream && stream,
        const vuu_block & block
        )
    {
      return FileStore::insert(stream, block);
    };

    inline
    static
    void
    insert(
        ostream && stream,
        const set<vuu_block> & record
        )
    {
      return FileStore::insert(stream, record);
    };

    static const string record_extension;
    static const string store_extension;

  private:

    const ConfigNode config;
    set<vuu_block> initial_record;
};

#endif
