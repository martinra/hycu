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

#include <random>

#include "store/file_store.hh"

namespace filesys = boost::filesystem;
using filesys::directory_iterator;
using filesys::is_regular_file;
using namespace std;


const string FileStore::record_extension = ".hycu_record";
const string FileStore::store_extension = ".hycu_store";

FileStore::
FileStore(
    const ConfigNode & config
    ) :
  config ( config )
{
  directory_iterator end_dir_iter;
  for ( directory_iterator dir_iter(config.result_path);
        dir_iter != end_dir_iter; ++dir_iter ) {
    path filepath(*dir_iter);
    if (    !is_regular_file(filepath)
         || filepath.extension() != FileStore::record_extension )
      continue;
    
    this->extract(filesys::fstream(filepath, ios_base::in) , this->initial_record);
  }
}

void
FileStore::
save(
    tuple<string, string> record_store
    )
{
  path path_record, path_store;
  tie(path_record, path_store) = this->new_filenames();

  // save record second as a witness to successful writing
  filesys::fstream(path_store,  ios_base::out) << get<1>(record_store);
  filesys::fstream(path_record, ios_base::out) << get<0>(record_store);
}

tuple<path, path>
FileStore::
new_filenames()
{
  mt19937_64 gen((random_device())());
  uniform_int_distribution<long int> dist;

  while ( true ) {
    stringstream id_ss, filename_store_ss, filename_record_ss;
    id_ss << dist(gen);
    filename_record_ss << id_ss.str() << ".hycu_record";
    filename_store_ss  << id_ss.str() << ".hycu_store";

    path path_record = this->full_path(filename_record_ss.str());
    path path_store = this->full_path(filename_store_ss.str());
    if ( is_regular_file(path_record) || is_regular_file(path_store) )
      continue;

    return make_tuple(path_record, path_store);
  }
};

istream &
FileStore::
extract(
    istream & stream,
    vuu_block & block
    )
{
  int lbd, ubd;

  string line;
  getline(stream, line);
  stringstream line_stream(line);

  while ( line_stream >> lbd >> ubd )
    block.push_back(make_tuple(lbd, ubd));

  return stream;
}

void
FileStore::
extract(
    istream & stream,
    set<vuu_block> & record
    )
{
  vuu_block block;

  stream.peek();
  while ( !stream.eof() ) {
    block.clear();
    FileStore::extract(stream, block);
    record.insert(block);
  }
}

ostream &
FileStore::
insert(
    ostream & stream,
    const vuu_block & block
    )
{
  for ( auto & bds : block )
    stream << get<0>(bds) << " " << get<1>(bds) << " ";
  return stream;
}

void
FileStore::
insert(
    ostream & stream,
    const set<vuu_block> & record
    )
{
  for ( auto & record_it : record )
    FileStore::insert(stream, record_it) << endl;
}

