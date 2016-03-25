#include <isogeny_representative_store.hh>


void
IsogenyRepresentativeStore::
register_curve(
    const Curve & curve
    )
{
  auto store_key =
    make_tuple( curve.ramifications(),
                curve.hasse_weil_offsets(curve.table->prime_exponent * curve.genus()) );
  auto store_it = this->store.find(store_key);
  if (store_it != this->store.end())
    this->store[store_key] = curve.poly_coeff_exponents;
}

ostream &
operator<<(
    ostream & stream,
    const IsogenyRepresentativeStore & store
    )
{
  for ( auto store_it : this->store ) {
    auto & ramifications = get<0>(store_it->first);
    auto & hasse_weil_offsets = get<1>(store_it->first);
    auto & poly_coeff_exponents = store_it->second;


    if ( !ramifications.empty() ) {
      stream << ramifications.front();
      for (size_t ix=1; ix<ramifications.size(); ++ix)
        stream << "," << ramifications[ix];
    }
    stream << ";";

    if ( !hasse_weil_offsets.empty() ) {
      stream << hasse_weil_offsets.front();
      for (size_t ix=1; ix<hasse_weil_offsets.size(); ++ix)
        stream << "," << hasse_weil_offsets[ix];
    }
    stream << ":";

    if ( !poly_coeff_exponents.empty() ) {
      stream << poly_coeff_exponents.front();
      for (size_t ix=1; ix<poly_coeff_exponents.size(); ++ix)
        stream << "," << poly_coeff_exponents[ix];
    }

    stream << endl;
  }

  return stream;
}
