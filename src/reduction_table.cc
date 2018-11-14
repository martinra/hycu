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


#include <cmath>
#include <flint/fq_nmod.h>
#include <flint/fmpz.h>
#include <flint/nmod_poly.h>
#include <flint/ulong_extras.h>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

#include "opencl/interface.hh"
#include "reduction_table.hh"


using namespace std;


ReductionTable::
ReductionTable(
    unsigned int prime,
    unsigned int prime_exponent,
    shared_ptr<OpenCLInterface> && opencl
    ) :
  prime( prime ),
  prime_exponent( prime_exponent ),
  prime_power( pow(prime,prime_exponent) ),
  prime_power_pred( prime_power - 1 ),
  opencl( move(opencl) )
{
  this->compute_tables();
#ifdef WITH_OPENCL
  if ( opencl ) {
    this->_buffer_evaluation = make_shared<OpenCLBufferEvaluation>(*this);
    this->_kernel_reduction = make_shared<OpenCLKernelReduction>(*this);
  }
#endif
}

ReductionTable::
ReductionTable(
    unsigned int prime,
    unsigned int prime_exponent,
    const shared_ptr<OpenCLInterface> & opencl
    ) :
  prime( prime ),
  prime_exponent( prime_exponent ),
  prime_power( pow(prime,prime_exponent) ),
  prime_power_pred( prime_power - 1 ),
  opencl( opencl )
{
  this->compute_tables();
#ifdef WITH_OPENCL
  if ( opencl ) {
    this->_buffer_evaluation = make_shared<OpenCLBufferEvaluation>(*this);
    this->_kernel_reduction = make_shared<OpenCLKernelReduction>(*this);
  }
#endif
}

void
ReductionTable::
compute_tables()
{
  this->exponent_reduction_table = this->compute_exponent_reduction_table(prime_power);
  this->incrementation_table =
      this->compute_incrementation_table(prime, prime_exponent, prime_power);
}

shared_ptr<vector<int32_t>>
ReductionTable::
compute_exponent_reduction_table(
    unsigned int prime_power
    )
{
  auto reductions = make_shared<vector<int32_t>>();
  reductions->reserve(2*(prime_power-1));

  for (size_t hx=0; hx<2; ++hx)
    for (size_t ix=0; ix<prime_power-1; ++ix)
      reductions->push_back(ix);

  return reductions;
}

shared_ptr<vector<int32_t>>
ReductionTable::
compute_incrementation_table(
    unsigned int prime,
    unsigned int prime_exponent,
    unsigned int prime_power
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


  auto incrementations = make_shared<vector<int32_t>>(prime_power);
  incrementations->at(prime_power-1) = 0; // special index for 0

  map<int32_t,int32_t> gen_powers;
  gen_powers[0] = prime_power - 1; // special index for 0


  fq_nmod_t a;
  fq_nmod_init(a, ctx);
  fq_nmod_one(a, ctx);

  for ( size_t ix=0; ix<prime_power-1; ++ix) {
    unsigned int coeff_sum = 0;
    for ( int dx= (int)prime_exponent-1; dx>=0; --dx ) {
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
