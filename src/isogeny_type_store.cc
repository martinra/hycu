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
register_poly(
    const vector<int> & poly_coeffs,
    const vector<tuple<int,int>> & nmb_points
    )
{
  vector<int> ramifications;

  int nmb_ramified = 0;
  for (size_t ix=0; ix<nmb_points.size(); ++ix) {
    for ( size_t jx=0;
          jx < get<1>(nmb_points[ix]) - nmb_ramified;
          jx+=ix+1 )
      ramifications.push_back(ix+1);
    nmb_ramified += get<1>(nmb_points[ix]);
  }
  int ramification_sum = accumulate(ramifications.cbegin(), ramifications.cend(), 0);

  int ramification_difference = poly_coeffs.size()-1 - ramification_sum;
  if (    ramification_difference != 0
       && ramification_difference < 2*(nmb_points.size()+1) )
    ramifications.push_back(ramification_difference);
  else {
    ramifications.clear();

    nmod_poly_factor_t poly_factor;
    nmod_poly_factor_init(poly_factor);

    if (nmod_poly_length(this->poly) != poly_coeffs.size())
      nmod_poly_realloc(this->poly, poly_coeffs.size());
    for (long ix=0; ix<poly_coeffs.size(); ++ix)
      nmod_poly_set_coeff_ui(this->poly, ix, poly_coeffs[ix]);

    nmod_poly_factor(poly_factor, this->poly);


    for (size_t ix=0; ix<poly_factor->num; ++ix)
      for (size_t jx=0; jx<poly_factor->exp[ix]; ++jx)
        ramifications.push_back(nmod_poly_degree(poly_factor->p + ix));

    sort(ramifications.begin(), ramifications.end());

    nmod_poly_factor_clear(poly_factor);
  } 


  vector<int> hasse_weil_offsets;
  for (size_t ix=0; ix<nmb_points.size(); ++ix)
    hasse_weil_offsets.push_back(
      pow(prime,ix+1) + 1 - (get<0>(nmb_points[ix]) + get<1>(nmb_points[ix])) );

  auto store_key = make_tuple(ramifications,hasse_weil_offsets);
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
