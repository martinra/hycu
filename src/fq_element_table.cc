#include <fq_element_table.hh>


FqElementTable::
FqElementTable(
    unsigned int prime,
    unsigned int prime_exponent
    ) :
  prime( prime ),
  prime_exponent( prime_exponent )
{
  this->prime_power = pow(prime, prime_exponent);
  this->prime_power_pred = this->prime_power - 1;

  fmpz_t prime_fmpz;
  fmpz_init_set_ui(prime_fmpz, prime);
  fq_nmod_ctx_init(this->fq_ctx, prime_fmpz, prime_exponent, ((string)"T").c_str());
  fmpz_clear(prime_fmpz);

  fq_nmod_t gen;
  fq_nmod_init(gen, this->fq_ctx);
  fq_nmod_gen(gen, this->fq_ctx);
  fq_nmod_reduce(gen, this->fq_ctx);

  fq_nmod_t a;
  fq_nmod_init(a, this->fq_ctx);
  fq_nmod_one(a, this->fq_ctx);

  this->fq_elements.reserve(this->prime_power-1);
  for (size_t ix=0; ix<this->prime_power-1; ++ix) {
    fq_nmod_mul(a, a, gen, this->fq_ctx);

    fq_nmod_t b;
    fq_nmod_init(b, this->fq_ctx);
    fq_nmod_set(b, a, this->fq_ctx);
    fq_nmod_reduce(b, this->fq_ctx);
    this->fq_elements.push_back(b);
  }
}

FqElementTable::
~FqElementTable()
{
  for ( auto fq : this->fq_elements )
    fq_nmod_clear(fq, this->fq_ctx);

  fq_nmod_ctx_clear(this->fq_ctx);
}


vector<unsigned int>
FqElementTable::
power_coset_representatives(
    unsigned int n
    )
{
  n = n_gcd(n, this->prime_power-1);

  vector<unsigned int> cosets;
  cosets.reserve(n);
  for (unsigned int i=0; i<n; ++i)
    cosets.push_back(i);

  return cosets;
}
