#include <cmath>
#include <flint/fq_nmod.h>
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
  for (size_t ix=curve.poly_coeff_exponents.size()-1; ix>=0; --ix) {
    // fixme: is cout.file() correct?
    fq_nmod_print_file(cout.file(), curve.table[curve.poly_coeff_exponents[ix]]);
    stream << "*X^" << ix;
    if (ix != 0) stream << " + ";
  }

  stream << "  /  " << "F_" << curve.table->prime_power;

  return stream;
}

Curve::
Curve(
    shared_ptr<EnumerationTable> table,
    const vector<int> poly_coeff_exponents
    ) :
    table( table )
{
  this->poly_coeff_exponents = move(poly_coeff_exponents);

  while (this->poly_coeff_exponents.back() == 0)
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
    nmod_poly_t poly = this->rhs_flint_polynomial();
    bool is_squarefree = nmod_poly_is_squarefree(poly);
    nmod_poly_clear(poly);

    return is_squarefree;
  }
  else {
    fq_nmod_poly_t poly = this->rhs_flint_polynomial();
    bool is_squarefree = fq_nmod_poly_is_squarefree(poly, this->table->fq_ctx);
    fq_nmod_poly_clear(poly, this->table->fq_ctx);

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

  if ( this->table->prime_exponent > table.prime_exponent ) {
    cerr << "Curve.convert_poly_coeff_exponents: Can not convert to lower prime exponent" << endl;
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

const vector<tuple<int,int>> &
Curve::
count(
    const ReductionTable & reduction_table
  )
{
  if ( reduction_table.prime != this->table->prime ) {
    cerr << "Curve.count: primes of curve and table must coincide" << endl;
    throw;
  }

  if ( reduction_table.prime_exponent <= this->nmb_points.size() )
    return vector<int>(this->nmb_points.cbegin(), this->nmb_points.cbegin() + reduction_table.prime_exponent);
  this->nmb_points.resize(resize.prime_exponent);


  auto opencl = reduction_table.opencl;

  // todo: Use error checking for all calls. Some implementations don't seem to support try/catch, but doublecheck this.
  int poly_size = (int)this->poly_coeff_exponents.size();
  vector<int> poly_coeffs_exponents = this->convert_poly_coeff_exponents(reduction_table);

  cl::Buffer buffer_poly_coeffs_exponents(
               *opencl.context, CL_MEM_READ_ONLY,
               sizeof(int) * poly_size);
  opencl.queue->enqueueWriteBuffer(buffer_poly_coeffs_exponents, CL_TRUE, 0,
                           sizeof(int) * poly_size, poly_coeffs_exponents.data());

  cl::Buffer buffer_nmbs_unramified(
               *opencl.context, CL_MEM_READ_WRITE, sizeof(int) * reduction_table.prime_power_pred);
  cl::Buffer buffer_nmbs_ramified(
               *opencl.context, CL_MEM_READ_WRITE, sizeof(int) * reduction_table.prime_power_pred);
  cl::Buffer buffer_minimal_fields(
               *opencl.context, CL_MEM_READ_WRITE, sizeof(int) * reduction_table.prime_power_pred);

  cl::Kernel kernel_evaluation = cl::Kernel(*opencl.program_evaluation, "evaluate");
  kernel_evaluation.setArg(0, buffer_poly_coeffs_exponents);
  kernel_evaluation.setArg(1, sizeof(int), &polysize);
  kernel_evaluation.setArg(2, sizeof(int), &reduction_table.prime_power_pred);
  kernel_evaluation.setArg(3, *reduction_table.buffer_exponent_reduction_table);
  kernel_evaluation.setArg(4, *reduction_table.buffer_incrementation_table);
  kernel_evaluation.setArg(4, *reduction_table.buffer_minimal_field_table);
  kernel_evaluation.setArg(5, buffer_nmbs_unramified);
  kernel_evaluation.setArg(6, buffer_nmbs_ramified);
  kernel_evaluation.setArg(7, buffer_minimal_fields);

  opencl.queue->enqueueNDRangeKernel( kernel_evaluation,
      cl::NullRange, cl::NDRange(reduction_table.prime_power_pred), cl::NullRange );


  const int global_size_reduction = 1024;
  const int local_size_reduction = 32;
  const int nmb_groups_reduction = global_size_reduction / local_size_reduction;

  cl::Buffer buffer_sums( *opencl.context, CL_MEM_WRITE_ONLY, sizeof(int) * nmb_groups_reduction);


  auto kernel_reduction = make_unique<cl::Kernel>(*opencl.program_reduction, "reduction");
  kernel_reduction->setArg(0, buffer_nmbs_unramified);
  kernel_reduction->setArg(1, buffer_minimal_fields);
  kernel_reduction->setArg(2, sizeof(int), &reduction_table.prime_power_pred);
  kernel_reduction->setArg(3, sizeof(int), &reduction_table.prime_exponent);
  kernel_reduction->setArg(4, sizeof(int) * reduction_table.prime_exponent, nullptr);
  kernel_reduction->setArg(5, sizeof(int) * local_size_reduction * reduction_table.prime_exponent, nullptr);
  kernel_reduction->setArg(6, buffer_sums);

  opencl.queue->enqueueNDRangeKernel(*kernel_reduction, cl::NullRange,
                             cl::NDRange(global_size_reduction), cl::NDRange(local_size_reduction));
  opencl.queue->finish();

  int * sums = new int[nmb_groups_reduction];
  opencl.queue->enqueueReadBuffer(buffer_sums, CL_TRUE, 0, sizeof(int)*nmb_groups_reduction, sums); 
  for (size_t fx=0; fx<reduction_table.prime_exponent; ++fx)
    for (size_t ix=fx*nmb_groups_reduction; ix<(fx+1)*nmb_groups_reduction; ++ix)
      get<0>(this->nmb_points[fx]) += sums[ix];


  kernel_reduction = make_unique<cl::Kernel>(*opencl.program_reduction, "reduction");
  kernel_reduction->setArg(0, buffer_nmbs_ramified);
  kernel_reduction->setArg(1, buffer_minimal_fields);
  kernel_reduction->setArg(2, sizeof(int), &reduction_table.prime_power_pred);
  kernel_reduction->setArg(3, sizeof(int), &reduction_table.prime_exponent);
  kernel_reduction->setArg(4, sizeof(int)*reduction_table.prime_exponent, nullptr);
  kernel_reduction->setArg(5, sizeof(int)*local_size_reduction*reduction_table.prime_exponent, nullptr);
  kernel_reduction->setArg(6, buffer_sums);

  opencl.queue->enqueueNDRangeKernel(*kernel_reduction, cl::NullRange,
                             cl::NDRange(global_size_reduction), cl::NDRange(local_size_reduction));
  opencl.queue->finish();

  int * sums = new int[nmb_groups_reduction];
  opencl.queue->enqueueReadBuffer(buffer_sums, CL_TRUE, 0, sizeof(int)*nmb_groups_reduction, sums); 
  for (size_t fx=0; fx<reduction_table.prime_exponent; ++fx)
    for (size_t ix=fx*nmb_groups_reduction; ix<(fx+1)*nmb_groups_reduction; ++ix)
      get<1>(this->nmb_points[fx]) += sums[ix];


  // point x = 0
  if (poly_coeffs_exponents.front() == table.prime_power - 1) // constant coefficient is zero
    get<1>(this->nmb_points[0])++;
  else if (!(poly_coeffs_exponents.front() & 1)) // constant coefficient is even power of generator
    get<0>(this->nmb_points[reduction_table.minimal_field_table[poly_coeffs_exponents.front()]]) += 2;

  // point x = infty
  if ( this->degree() < 2*this->genus() + 2 ) // poly_coeffs ends with zero entry
    get<1>(this->nmb_points[0]) += 1;
  else if (!(poly_coeffs_exponents.back() & 1)) // leading coefficient is odd power of generator
    get<0>(this->nmb_points[reduction_table.minimal_field_table[poly_coeffs_exponents.back()]]) += 2;


  return this->nmb_points;
}


vector<int>
Curve::
hasse_weil_offsets()
{
  if ( !this->table->is_prime_field() ) {
    cerr << "hasse_weil_offsets implemented only for prime fields" << endl;
    throw;
    // todo: implement
  }

  vector<int> hasse_weil_offsets();
  hasse_weil_offsets.reserve(this->nmb_points.size());
  for (size_t ix=0; ix<nmb_points.size(); ++ix)
    hasse_weil_offsets.push_back(
      pow(this->table->prime, ix+1) + 1 - (get<0>(this->nmb_points[ix]) + get<1>(this->nmb_points[ix])) );
}


vector<int>
Curve::
ramification_type()
{
  if ( !this->table->is_prime_field() ) {
    cerr << "ramification_type implemented only for prime fields" << endl;
    throw;
    // todo: implement
  }

  vector<int> ramifications;

  for (size_t ix=0; ix<nmb_points.size(); ++ix)
    for ( size_t jx=0; jx < get<1>(this->nmb_points[ix]); jx+=ix+1 )
      ramifications.push_back(ix+1);
  int ramification_sum = accumulate(ramifications.cbegin(), ramifications.cend(), 0);


  int ramification_difference = this->degree() - ramification_sum;
  if ( ramification_difference < 2*(this->nmb_points.size()+1) ) {
    if ( ramification_difference != 0 )
      ramifications.push_back(ramification_difference);
    return ramifications;
  }


  ramifications.clear();

  nmod_poly_t poly = this->rhs_flint_polynomial();
  nmod_poly_factor_t poly_factor;
  nmod_poly_factor_init(poly_factor);
  nmod_poly_factor(poly_factor, poly);

  for (size_t ix=0; ix<poly_factor->num; ++ix)
    for (size_t jx=0; jx<poly_factor->exp[ix]; ++jx)
      ramifications.push_back(nmod_poly_degree(poly_factor->p + ix));

  nmod_poly_factor_clear(poly_factor);
  nmod_poly_clear(poly);

  sort(ramifications.begin(), ramifications.end());
  return ramifications;
}

nmod_poly_t
Curve::
rhs_flint_polynomial()
{
  if ( this->table->prime_exponent != 1 ) {
    cerr << "Conversion to nmod_poly_t only possible for prime fields" << endl;
    throw;
  }

  nmod_poly_t poly;
  nmod_poly_init2_preinv( poly, this->table->prime, this->table->primeinv_flint,
                          this->poly_coeffs_exponents.size() );

  for (long ix=0; ix<this->poly_coeff_exponents.size(); ++ix)
    nmod_poly_set_coeff_ui(poly, ix, (*this->table)[this->poly_coeff_exponents[ix]]);

  return poly;
}

fq_nmod_poly_t
Curve::
rhs_flint_polynomial()
{
  fq_nmod_poly_t poly;
  fq_nmod_poly_init2( poly, this->table->prime, this->poly_coeffs_exponents.size(), this->table->fq_ctx );

  for (long ix=0; ix<this->poly_coeff_exponents.size(); ++ix)
    fq_nmod_poly_set_coeff( poly, ix, (*this->table)[this->poly_coeff_exponents[ix]], this->table->fq_ctx );

  return poly;
}
