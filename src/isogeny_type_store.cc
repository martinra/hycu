#include <algorithm>
#include <numeric>
#include <iostream>

#include <isogeny_type_store.hh>


IsogenyTypeStore::
IsogenyTypeStore(
    int prime
    ) :
  prime( prime )
{
  nmod_poly_init(this->poly, prime);
}

IsogenyTypeStore::
~IsogenyTypeStore()
{
  nmod_poly_clear(this->poly);
}

void
IsogenyTypeStore::
register_curve(
    const Curve & curve
    )
{
  if ( !curve.table->is_prime_field() ) {
    cerr << "register_curve implemented only for prime fields" << endl;
    throw;
    // todo: implement
  }

  auto store_key = make_tuple(curve.ramifications(),curve.hasse_weil_offsets(curve.genus()));
  auto store_it = this->store.find(store_key);
  if (store_it == this->store.end())
    this->store[store_key] = 1;
  else
    store_it->second += 1;
}

int
IsogenyTypeStore::
to_legacy_ramification(
    const vector<int> & ramifications
    )
{
  if (!this->legacy_ramfication_vectors) {
    this->legacy_ramfication_vectors = make_shared<map<vector<int>,int>>();
    (*this->legacy_ramfication_vectors)[vector<int>({1,1,1,1,1,1})] = 0;
    (*this->legacy_ramfication_vectors)[vector<int>({1,1,1,1,2})] = 1;
    (*this->legacy_ramfication_vectors)[vector<int>({1,1,2,2})] = 2;
    (*this->legacy_ramfication_vectors)[vector<int>({2,2,2})] = 3;
    (*this->legacy_ramfication_vectors)[vector<int>({1,1,1,3})] = 4;
    (*this->legacy_ramfication_vectors)[vector<int>({1,2,3})] = 5;
    (*this->legacy_ramfication_vectors)[vector<int>({1,1,3})] = 6;
    (*this->legacy_ramfication_vectors)[vector<int>({1,1,4})] = 6;
    (*this->legacy_ramfication_vectors)[vector<int>({2,3})] = 7;
    (*this->legacy_ramfication_vectors)[vector<int>({2,4})] = 7;
    (*this->legacy_ramfication_vectors)[vector<int>({1,4})] = 8;
    (*this->legacy_ramfication_vectors)[vector<int>({1,5})] = 8;
    (*this->legacy_ramfication_vectors)[vector<int>({6})] = 9;
    (*this->legacy_ramfication_vectors)[vector<int>({3,3})] = 10;
  }

  auto legacy_it = this->legacy_ramfication_vectors->find(ramifications);

  if ( legacy_it != this->legacy_ramfication_vectors->end() )
    return legacy_it->second;
  else {
    cerr << "unrecognized legacy ramification type";
    for ( auto r : ramifications )
      cerr << " " << r;
    cerr << endl;
    throw;
  }
}

ostream &
IsogenyTypeStore::
output_legacy(
    ostream & stream
    )
{
  for (auto const & store_it : this->store) {
    auto & ramifications = get<0>(store_it.first);
    auto & hasse_weil_offsets = get<1>(store_it.first);

    stream << "[" << this->to_legacy_ramification(ramifications) << ", ";

    if (!hasse_weil_offsets.empty()) {
      stream << hasse_weil_offsets.front();
      for ( auto offset_it = hasse_weil_offsets.cbegin()+1;
            offset_it != hasse_weil_offsets.cend();
            ++offset_it )
        stream << "," << (int)(*offset_it);
    }

    stream << "]=" << store_it.second << endl;
  }

  return stream;
}
