#include <cmath>
#include <flint/fq_nmod.h>
#include <flint/fmpz.h>
#include <flint/nmod_poly.h>
#include <flint/ulong_extras.h>
#include <map>
#include <memory>
#include <tuple>
#include <vector>
#include <CL/cl.hpp>

#include <opencl_interface.hh>
#include <reduction_table.hh>

using namespace std;


ReductionTable::
ReductionTable(
    int prime,
    int prime_exponent,
    OpenCLInterface & opencl
    ) :
  prime( prime ),
  prime_exponent( prime_exponent ),
  prime_power( pow(prime,prime_exponent) ),
  prime_power_pred( prime_power - 1 )
{
  this->exponent_reduction_table = this->compute_exponent_reduction_table(prime_power);
  this->incrementation_table =
      this->compute_incrementation_table(prime, prime_exponent, prime_power);
  this->minimal_field_table =
      this->compute_minimal_field_table(prime, prime_exponent, prime_power);

  this->buffer_exponent_reduction_table = make_shared<cl::Buffer>(
      *opencl.context, CL_MEM_READ_ONLY, sizeof(int) * this->exponent_reduction_table->size() );
  this->buffer_incrementation_table = make_shared<cl::Buffer>(
      *opencl.context, CL_MEM_READ_ONLY, sizeof(int) * this->incrementation_table->size() );
  this->buffer_minimal_field_table = make_shared<cl::Buffer>(
      *opencl.context, CL_MEM_READ_ONLY, sizeof(int) * this->minimal_field_table->size() );

  opencl.queue->enqueueWriteBuffer(*this->buffer_exponent_reduction_table, CL_TRUE, 0,
      sizeof(int)*this->exponent_reduction_table->size(),
      this->exponent_reduction_table->data() );
  opencl.queue->enqueueWriteBuffer(*this->buffer_incrementation_table, CL_TRUE, 0,
      sizeof(int)*this->incrementation_table->size(),
      this->incrementation_table->data() );
  opencl.queue->enqueueWriteBuffer(*this->buffer_minimal_field_table, CL_TRUE, 0,
      sizeof(int)*this->minimal_field_table->size(),
      this->minimal_field_table->data() );
}

shared_ptr<vector<int>>
ReductionTable::
compute_exponent_reduction_table(
    int prime_power
    )
{
  auto reductions = make_shared<vector<int>>();
  reductions->reserve(2*(prime_power-1));

  for (size_t hx=0; hx<2; ++hx)
    for (size_t ix=0; ix<prime_power-1; ++ix)
      reductions->push_back(ix);

  return reductions;
}

shared_ptr<vector<int>>
ReductionTable::
compute_incrementation_table(
    int prime,
    int prime_exponent,
    int prime_power
    )
{
  fmpz_t prime_fmpz;
  fmpz_init(prime_fmpz);
  fmpz_set_si(prime_fmpz, prime);
  fq_nmod_ctx_t ctx;
  fq_nmod_ctx_init(ctx, prime_fmpz, prime_exponent, ((string)"T").c_str());
  fmpz_clear(prime_fmpz);

  fq_nmod_t gen;
  fq_nmod_init(gen, ctx);
  fq_nmod_gen(gen, ctx);
  fq_nmod_reduce(gen, ctx);


  auto incrementations = make_shared<vector<int>>(prime_power);
  incrementations->at(prime_power-1) = 0; // special index for 0

  map<int,int> gen_powers;
  gen_powers[0] = prime_power - 1; // special index for 0


  fq_nmod_t a;
  fq_nmod_init(a, ctx);
  fq_nmod_one(a, ctx);

  for ( size_t ix=0; ix<prime_power-1; ++ix) {
    unsigned int coeff_sum = 0;
    for ( int dx=prime_exponent-1; dx>=0; --dx ) {
      coeff_sum *= prime;
      coeff_sum += nmod_poly_get_coeff_ui(a,dx);
    }

    gen_powers[coeff_sum] = ix;

    fq_nmod_mul(a, a, gen, ctx);
    fq_nmod_reduce(a, ctx);
  }

  for ( size_t pix=0; pix<prime_power-1; pix+=prime) {
    for ( size_t ix=pix; ix<pix+prime-1; ++ix)
      incrementations->at(gen_powers[ix]) = gen_powers[ix+1];
    incrementations->at(gen_powers[pix+prime-1]) = gen_powers[pix];
  }


  fq_nmod_clear(gen, ctx); 
  fq_nmod_clear(a, ctx); 
  fq_nmod_ctx_clear(ctx);

  return incrementations;
}

shared_ptr<vector<int>>
ReductionTable::
compute_minimal_field_table(
    int prime,
    int prime_exponent,
    int prime_power
    )
{
  auto minimal_field = make_shared<vector<int>>(prime_power, prime_exponent-1);

  minimal_field[prime_power-1] = 0;
  for ( int ex=prime_exponent-2; ex>=0; --ex ) {
    if ( prime_exponent % (ex+1) != 0 ) continue;

    int factor_exponent = (prime_power - 1) / (pow(prime, ex+1) - 1);
    for ( int ix=0; ix<prime_power-1; ix += factor_exponent )
      minimal_field[ix] = ex;
  }

  return minimal_field;
}
