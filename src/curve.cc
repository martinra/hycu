#include <algorithm>
#include <cmath>
#include <flint/fq_nmod.h>
#include <flint/fq_nmod_poly.h>
#include <flint/fmpz.h>
#include <flint/nmod_poly.h>
#include <map>
#include <memory>
#include <numeric>
#include <iostream>
#include <string>
#include <sstream>
#include <tuple>
#include <vector>
#include <CL/cl.hpp>

#include <opencl_interface.hh>
#include <reduction_table.hh>
#include <curve.hh>


using namespace std;


ostream&
operator<<(
    ostream & stream,
    const Curve & curve
    )
{
  stream << "Y^2 = ";
  for (int ix=curve.poly_coeff_exponents.size()-1; ix>=0; --ix) {
    auto coeff_str = fq_nmod_get_str_pretty(curve.table->at(curve.poly_coeff_exponents[ix]), curve.table->fq_ctx);
    stream << "(" << coeff_str << ")*X^" << ix;
    flint_free(coeff_str);
    if (ix != 0) stream << " + ";
  }

  stream << "  /  " << "F_" << curve.table->prime_power;

  return stream;
}

Curve::
Curve(
    shared_ptr<FqElementTable> table,
    const vector<int> poly_coeff_exponents
    ) :
    table( table )
{
  this->poly_coeff_exponents = move(poly_coeff_exponents);

  while ( this->poly_coeff_exponents.back() == this->table->zero_index() )
    this->poly_coeff_exponents.pop_back();
}

int
Curve::
genus()
  const
{
  if (this->degree() % 2 == 0)
    return (this->degree() - 2) / 2;
  else
    return (this->degree() - 1) / 2;
}

bool
Curve::
has_squarefree_rhs()
{
  if ( this->table->is_prime_field() ) {
    auto poly = this->rhs_nmod_polynomial();
    bool is_squarefree = nmod_poly_is_squarefree(&poly);
    nmod_poly_clear(&poly);

    return is_squarefree;
  }
  else {
    auto poly = this->rhs_polynomial();
    bool is_squarefree = fq_nmod_poly_is_squarefree(&poly, this->table->fq_ctx);
    fq_nmod_poly_clear(&poly, this->table->fq_ctx);

    return is_squarefree;
  }
}

vector<int>
Curve::
convert_poly_coeff_exponents(
    const ReductionTable & table
    )
{
  if ( this->table->prime != table.prime ) {
    cerr << "Curve.convert_poly_coeff_exponents: Can only convert to same prime" << endl;
    throw;
  }

  if ( table.prime_exponent % this->table->prime_exponent != 0  ) {
    cerr << "Curve.convert_poly_coeff_exponents: Can not convert to prime exponent "
         << "that does not divide the one of the curve" << endl;
    throw;
  }
  else if ( this->table->prime_exponent == table.prime_exponent ) {
    return this->poly_coeff_exponents;
  }

  unsigned int prime_power_pred = this->table->prime_power_pred;
  unsigned int exponent_factor = table.prime_power_pred / this->table->prime_power_pred;

  vector<int> converted;
  converted.reserve(this->poly_coeff_exponents.size());
  for ( int c : this->poly_coeff_exponents )
    converted.push_back(c != prime_power_pred ? exponent_factor * c
                                              : table.prime_power_pred );

  return converted;
}

void
Curve::
count(
    const ReductionTable & reduction_table
  )
{
  // todo: Use error checking for all calls. Some implementations don't seem to support try/catch, but doublecheck this.

  if ( reduction_table.prime != this->table->prime ) {
    cerr << "Curve.count: primes of curve and table must coincide" << endl;
    throw;
  }

  if ( this->nmb_points.find( reduction_table.prime_exponent ) != this->nmb_points.end() )
    return;


  int prime_exponent = reduction_table.prime_exponent;
  int prime_power_pred = reduction_table.prime_power_pred;

  // this also checks that the prime exponent is divisible by the one of the curve
  int poly_size = (int)this->poly_coeff_exponents.size();
  vector<int> poly_coeff_exponents = this->convert_poly_coeff_exponents(reduction_table);


  auto opencl = reduction_table.opencl;

  cl::Buffer buffer_poly_coeff_exponents(
               *opencl->context, CL_MEM_READ_ONLY,
               sizeof(int) * poly_size);
  opencl->queue->enqueueWriteBuffer(buffer_poly_coeff_exponents, CL_TRUE, 0,
                           sizeof(int) * poly_size, poly_coeff_exponents.data());


  cl::Buffer buffer_nmbs_unramified(
               *opencl->context, CL_MEM_READ_WRITE, sizeof(int) * reduction_table.prime_power_pred);
  cl::Buffer buffer_nmbs_ramified(
               *opencl->context, CL_MEM_READ_WRITE, sizeof(int) * reduction_table.prime_power_pred);
  cl::Buffer buffer_minimal_fields(
               *opencl->context, CL_MEM_READ_WRITE, sizeof(int) * reduction_table.prime_power_pred);

  cl::Kernel kernel_evaluation(*opencl->program_evaluation, "evaluate");
  kernel_evaluation.setArg(0, buffer_poly_coeff_exponents);
  kernel_evaluation.setArg(1, sizeof(int), &poly_size);
  kernel_evaluation.setArg(2, sizeof(int), &prime_power_pred);
  kernel_evaluation.setArg(3, *reduction_table.buffer_exponent_reduction_table);
  kernel_evaluation.setArg(4, *reduction_table.buffer_incrementation_table);
  kernel_evaluation.setArg(5, *reduction_table.buffer_minimal_field_table);
  kernel_evaluation.setArg(6, buffer_nmbs_unramified);
  kernel_evaluation.setArg(7, buffer_nmbs_ramified);
  kernel_evaluation.setArg(8, buffer_minimal_fields);

  opencl->queue->enqueueNDRangeKernel( kernel_evaluation,
      cl::NullRange, cl::NDRange(reduction_table.prime_power_pred), cl::NullRange );


  const int global_size_reduction = 1024;
  const int local_size_reduction = 32;
  const int nmb_groups_reduction = global_size_reduction / local_size_reduction;

  cl::Buffer buffer_sums(*opencl->context, CL_MEM_WRITE_ONLY, sizeof(int) * nmb_groups_reduction * prime_exponent);

  auto kernel_reduction = make_unique<cl::Kernel>(*opencl->program_reduction, "reduce");
  kernel_reduction->setArg(0, buffer_nmbs_unramified);
  kernel_reduction->setArg(1, buffer_minimal_fields);
  kernel_reduction->setArg(2, sizeof(int), &prime_power_pred);
  kernel_reduction->setArg(3, sizeof(int), &prime_exponent);
  kernel_reduction->setArg(4, sizeof(int) * local_size_reduction * prime_exponent, nullptr);
  kernel_reduction->setArg(5, buffer_sums);

 opencl->queue->enqueueNDRangeKernel( *kernel_reduction,
     cl::NullRange, cl::NDRange(global_size_reduction), cl::NDRange(local_size_reduction) );


  int * sums = new int[nmb_groups_reduction*prime_exponent];
  opencl->queue->enqueueReadBuffer( buffer_sums,
                                    CL_TRUE, 0, sizeof(int) * nmb_groups_reduction * prime_exponent, sums);
  opencl->queue->finish();

  for (size_t fx=1; fx<=prime_exponent; ++fx)
    if ( prime_exponent % fx == 0 )
      for (size_t ix=(fx-1)*nmb_groups_reduction; ix<fx*nmb_groups_reduction; ++ix)
        get<0>(this->nmb_points[fx]) += sums[ix];

  delete sums;


  kernel_reduction = make_unique<cl::Kernel>(*opencl->program_reduction, "reduce");
  kernel_reduction->setArg(0, buffer_nmbs_ramified);
  kernel_reduction->setArg(1, buffer_minimal_fields);
  kernel_reduction->setArg(2, sizeof(int), &prime_power_pred);
  kernel_reduction->setArg(3, sizeof(int), &prime_exponent);
  kernel_reduction->setArg(4, sizeof(int) * local_size_reduction * prime_exponent, nullptr);
  kernel_reduction->setArg(5, buffer_sums);

  opencl->queue->enqueueNDRangeKernel( *kernel_reduction,
      cl::NullRange, cl::NDRange(global_size_reduction), cl::NDRange(local_size_reduction) );


  sums = new int[nmb_groups_reduction*prime_exponent];
  opencl->queue->enqueueReadBuffer( buffer_sums,
      CL_TRUE, 0, sizeof(int) * nmb_groups_reduction * prime_exponent, sums); 
  opencl->queue->finish();

  for (size_t fx=1; fx<=prime_exponent; ++fx)
    if ( prime_exponent % fx == 0 )
      for (size_t ix=(fx-1)*nmb_groups_reduction; ix<fx*nmb_groups_reduction; ++ix)
        get<1>(this->nmb_points[fx]) += sums[ix];

  delete sums;


  // point x = 0
  // if constant coefficient is zero
  if (poly_coeff_exponents.front() == reduction_table.prime_power_pred)
    for ( size_t fx=1; fx<=prime_exponent; ++fx)
      ++get<1>(this->nmb_points[fx]);
  // if constant coefficient is even power of generator
  else if (!(poly_coeff_exponents.front() & 1)) {
    size_t fx_min = (*reduction_table.minimal_field_table)[poly_coeff_exponents.front() / 2] + 1;
    for ( size_t fx=fx_min; fx<=prime_exponent; fx+=fx_min )
      if ( prime_exponent % fx == 0 )
        get<0>(this->nmb_points[fx]) += 2;
  }


  // point x = infty
  // if poly_coeffs ends with zero entry
  if ( this->degree() < 2*this->genus() + 2 )
    for ( size_t fx=1; fx<=prime_exponent; ++fx)
      get<1>(this->nmb_points[fx]) += 1;
  // f leading coefficient is odd power of generator
  else if (!(poly_coeff_exponents.back() & 1)) {
    // todo: make sure that curve enumeration does not put 0 as the leading coefficient
    size_t fx_min = (*reduction_table.minimal_field_table)[poly_coeff_exponents.back() / 2] + 1;
    for ( size_t fx=fx_min; fx<=prime_exponent; fx+=fx_min )
      if ( prime_exponent % fx == 0 )
        get<0>(this->nmb_points[fx]) += 2;
  }
}

vector<tuple<int,int>>
Curve::
number_of_points(
    unsigned int max_prime_exponent
    )
  const
{
  unsigned int prime_exponent = this->table->prime_exponent;

  vector<tuple<int,int>> nmb_points;
  nmb_points.reserve(max_prime_exponent/prime_exponent);
  for ( size_t fx=prime_exponent;
        fx<=max_prime_exponent;
        fx+=prime_exponent )
    nmb_points.push_back(this->nmb_points.at(fx));

  return nmb_points;
}

map<unsigned int, int>
Curve::
hasse_weil_offsets()
  const
{
  if ( !this->table->is_prime_field() ) {
    cerr << "hasse_weil_offsets implemented only for prime fields" << endl;
    throw;
    // todo: implement
  }

  map<unsigned int, int> offsets;
  for ( auto & pts_it : this->nmb_points )
    offsets[pts_it.first] =   pow(this->table->prime, pts_it.first) + 1
                            - (get<0>(pts_it.second) + get<1>(pts_it.second));

  return offsets;
}

vector<int>
Curve::
hasse_weil_offsets(
    unsigned int max_prime_exponent
    )
  const
{
  unsigned int prime_exponent = this->table->prime_exponent;
  auto offset_map = this->hasse_weil_offsets();

  vector<int> offsets;
  offsets.reserve(max_prime_exponent / prime_exponent);
  for ( size_t fx=prime_exponent;
        fx<=max_prime_exponent;
        fx+=prime_exponent )
    offsets.push_back(offset_map[fx]);

  return offsets;
}

vector<int>
Curve::
ramification_type()
  const
{
  vector<int> ramifications;

  // try to compute ramification from point counts

  unsigned int ramification_sum = 0;
  map<unsigned int, unsigned int> nmb_ramified_points_from_lower;
  for ( unsigned int fx = 1; fx < this->degree(); ++fx )
    nmb_ramified_points_from_lower[fx] = 0;

  unsigned int fx;
  for ( fx = 1; fx < this->degree(); ++fx ) {
    auto nmb_points_it = this->nmb_points.find(fx*this->prime_exponent());
    if ( nmb_points_it != this->nmb_points.end() ) {
      auto nmb_ramified_points_new =
          get<1>(nmb_points_it->second) - nmb_ramified_points_from_lower[fx];

      ramification_sum += nmb_ramified_points_new;
      for ( size_t jx = 0; jx < nmb_ramified_points_new; jx += fx )
        ramifications.push_back(fx);

      for ( size_t gx = fx; gx < this->degree(); gx += fx )
        nmb_ramified_points_from_lower[gx] += nmb_ramified_points_new;
    }
    else
      break;
  }


  int ramification_difference = this->degree() - ramification_sum;
  if ( ramification_difference < 2*fx ) {
    if ( ramification_difference != 0 )
      ramifications.push_back(ramification_difference);
    return ramifications;
  }


  // if ramification can not be computed from available point count,
  // factor the right hand side polynomial

  ramifications.clear();
  if ( this->table->is_prime_field() ) {
    auto poly = this->rhs_nmod_polynomial();
    nmod_poly_factor_t poly_factor;
    nmod_poly_factor_init(poly_factor);
    nmod_poly_factor(poly_factor, &poly);

    for (size_t ix=0; ix<poly_factor->num; ++ix)
      for (size_t jx=0; jx<poly_factor->exp[ix]; ++jx)
        ramifications.push_back(nmod_poly_degree(poly_factor->p + ix));

    nmod_poly_factor_clear(poly_factor);
    nmod_poly_clear(&poly);
  }
  else {
    auto poly = this->rhs_polynomial();
    fq_nmod_t lead;
    fq_nmod_init(lead, this->table->fq_ctx);
    fq_nmod_poly_factor_t poly_factor;
    fq_nmod_poly_factor_init(poly_factor, this->table->fq_ctx);
    fq_nmod_poly_factor(poly_factor, lead, &poly, this->table->fq_ctx);

    for (size_t ix=0; ix<poly_factor->num; ++ix)
      for (size_t jx=0; jx<poly_factor->exp[ix]; ++jx)
        ramifications.push_back(fq_nmod_poly_degree(poly_factor->poly + ix, this->table->fq_ctx));

    fq_nmod_poly_factor_clear(poly_factor, this->table->fq_ctx);
    fq_nmod_poly_clear(&poly, this->table->fq_ctx);
    fq_nmod_clear(lead, this->table->fq_ctx);
  }

  sort(ramifications.begin(), ramifications.end());
  return ramifications;
}

nmod_poly_struct
Curve::
rhs_nmod_polynomial()
  const
{
  if ( this->table->prime_exponent != 1 ) {
    cerr << "Conversion to nmod_poly_t only possible for prime fields" << endl;
    throw;
  }

  nmod_poly_struct poly;
  nmod_poly_init2_preinv( &poly, this->table->prime, this->table->primeinv_flint,
                          this->poly_coeff_exponents.size() );

  for (long ix=0; ix<this->poly_coeff_exponents.size(); ++ix)
    nmod_poly_set_coeff_ui(&poly, ix, this->table->at_nmod(this->poly_coeff_exponents[ix]));

  return poly;
}

fq_nmod_poly_struct
Curve::
rhs_polynomial()
  const
{
  fq_nmod_poly_struct poly;
  fq_nmod_poly_init2( &poly, this->poly_coeff_exponents.size(), this->table->fq_ctx );

  for (long ix=0; ix<this->poly_coeff_exponents.size(); ++ix)
    fq_nmod_poly_set_coeff( &poly, ix, this->table->at(this->poly_coeff_exponents[ix]), this->table->fq_ctx );

  return poly;
}
