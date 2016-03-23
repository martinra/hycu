#include <cmath>
#include <flint/fq_nmod.h>
#include <flint/fmpz.h>
#include <flint/nmod_poly.h>
#include <map>
#include <memory>
#include <numeric>
#include <iostream>
#include <set>
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
  for (size_t ix=curve.poly_coeffs.size()-1; ix>0; --ix) {
    if (ix != 0) stream << " + ";
    stream << curve.poly_coeffs[ix] << "*X^" << ix;
  }

  stream << "  /  " << "F_" << curve.prime;

  return stream;
}

Curve::
Curve(
    int prime,
    const vector<int>& poly_coeffs
    ) :
    prime( prime )
{
  this->poly_coeffs = poly_coeffs;
  while (!this->poly_coeffs.back())
    this->poly_coeffs.pop_back();
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
  nmod_poly_t poly;
  nmod_poly_init2(poly, this->prime, poly_coeffs.size());

  for (size_t ix=0; ix<this->poly_coeffs.size(); ++ix)
    nmod_poly_set_coeff_ui(poly, ix, this->poly_coeffs[ix]);
  bool is_squarefree = nmod_poly_is_squarefree(poly);

  nmod_poly_clear(poly);

  return is_squarefree;
}

vector<int>
Curve::
poly_coefficients_as_generator_exponents(
    const ReductionTable & table
    )
{
  vector<int> converted;
  converted.reserve(this->poly_coeffs.size());

  for ( int c : this->poly_coeffs )
    converted.push_back(table.fp_exponents->at(c));

  return converted;
}
tuple<int,int>
Curve::
count(
    const ReductionTable & table,
    const OpenCLInterface & opencl
  )
{
  // todo: Use error checking for all calls. Some implementations don't seem to support try/catch, but doublecheck this.
  int poly_coeffs_size = (int)this->poly_coeffs.size();
  vector<int> poly_coeffs_exponents = this->poly_coefficients_as_generator_exponents(table);
  int nmb_units = table.prime_power-1;

  cl::Buffer buffer_poly_coeffs_exponents(
               *opencl.context, CL_MEM_READ_ONLY,
               sizeof(int) * poly_coeffs_size);
  opencl.queue->enqueueWriteBuffer(buffer_poly_coeffs_exponents, CL_TRUE, 0,
                           sizeof(int) * poly_coeffs_size, poly_coeffs_exponents.data());

  cl::Buffer buffer_nmbs_unramified(
               *opencl.context, CL_MEM_READ_WRITE, sizeof(int) * nmb_units);
  cl::Buffer buffer_nmbs_ramified(
               *opencl.context, CL_MEM_READ_WRITE, sizeof(int) * nmb_units);

  cl::Kernel kernel_evaluation = cl::Kernel(*opencl.program_evaluation, "evaluate");
  kernel_evaluation.setArg(0, sizeof(int), &poly_coeffs_size);
  kernel_evaluation.setArg(1, buffer_poly_coeffs_exponents);
  kernel_evaluation.setArg(2, sizeof(int), &nmb_units);
  kernel_evaluation.setArg(3, *table.buffer_exponent_reduction_table);
  kernel_evaluation.setArg(4, *table.buffer_incrementation_table);
  kernel_evaluation.setArg(5, buffer_nmbs_unramified);
  kernel_evaluation.setArg(6, buffer_nmbs_ramified);

  opencl.queue->enqueueNDRangeKernel(kernel_evaluation, cl::NullRange, cl::NDRange(nmb_units), cl::NullRange);


  const int global_size_reduction = 1024;
  const int local_size_reduction = 32;
  const int nmb_groups_reduction = global_size_reduction / local_size_reduction;

  cl::Buffer buffer_sums_nmbs_unramified(
               *opencl.context, CL_MEM_WRITE_ONLY,
               sizeof(int) * nmb_groups_reduction);
  cl::Buffer buffer_sums_nmbs_ramified(
               *opencl.context, CL_MEM_WRITE_ONLY,
               sizeof(int) * nmb_groups_reduction);

  cl::Kernel kernel_reduction = cl::Kernel(*opencl.program_reduction, "reduction");
  kernel_reduction.setArg(0, buffer_nmbs_unramified);
  kernel_reduction.setArg(1, buffer_nmbs_ramified);
  kernel_reduction.setArg(2, sizeof(int)*local_size_reduction, nullptr);
  kernel_reduction.setArg(3, sizeof(int)*local_size_reduction, nullptr);
  kernel_reduction.setArg(4, sizeof(int), &nmb_units);
  kernel_reduction.setArg(5, buffer_sums_nmbs_unramified);
  kernel_reduction.setArg(6, buffer_sums_nmbs_ramified);

  opencl.queue->enqueueNDRangeKernel(kernel_reduction, cl::NullRange,
                             cl::NDRange(global_size_reduction), cl::NDRange(local_size_reduction));
  opencl.queue->finish();


  int * sums_nmbs_unramified = new int[nmb_groups_reduction];
  int * sums_nmbs_ramified = new int[nmb_groups_reduction];
  opencl.queue->enqueueReadBuffer(buffer_sums_nmbs_unramified, CL_TRUE, 0,
                          sizeof(int)*nmb_groups_reduction, sums_nmbs_unramified);
  opencl.queue->enqueueReadBuffer(buffer_sums_nmbs_ramified, CL_TRUE, 0,
                          sizeof(int)*nmb_groups_reduction, sums_nmbs_ramified);

  int nmb_unramified = 0, nmb_ramified = 0;
  for (size_t ix=0; ix<nmb_groups_reduction; ++ix) {
    nmb_unramified += sums_nmbs_unramified[ix];
    nmb_ramified += sums_nmbs_ramified[ix];
  }

  // point x = 0
  if (poly_coeffs_exponents[0] == table.prime_power - 1) // constant coefficient is zero
    nmb_ramified++;
  else if (!(poly_coeffs_exponents[0] & 1)) // constant coefficient is even power of generator
    nmb_unramified += 2;

  // point x = infty
  if ( this->degree() < 2*this->genus() + 2 ) // poly_coeffs ends with zero entry
    nmb_ramified += 1;
  else if (!(poly_coeffs_exponents.back() & 1)) // leading coefficient is odd power of generator
    nmb_unramified += 2;


  return make_pair(nmb_unramified, nmb_ramified);
}

vector<tuple<int,int>>
Curve::
isogeny_nmb_points(
    const vector<ReductionTable>& tables,
    const OpenCLInterface& opencl
    )
{
  auto nmb_points = vector<tuple<int,int>>();
  nmb_points.reserve(tables.size());

  for (const ReductionTable & table : tables)
    nmb_points.push_back(this->count(table, opencl));

  return nmb_points;
}


CurveCounter::
CurveCounter(
    int prime,
    vector<tuple<int,int>> coeff_bounds
    ) :
  prime( prime ), coeff_bounds( coeff_bounds )
{
  int lbd, ubd;

  for ( auto it = this->coeff_bounds.rbegin();
        it != this->coeff_bounds.rend();
        ++it ) {
    tie(lbd,ubd) = *it;

    if (lbd >= ubd) {
      cerr << "CurveCounter constructor: lower greater equal upper bound" << endl;
      throw;
    }
    if (lbd < 0 || ubd > prime) {
      cerr << "CurveCounter constructor: lower < 0 or upper > prime" << endl;
      throw;
    }
  }

  this->genus_ = coeff_bounds.size() % 2 == 0 ? (coeff_bounds.size() - 3) / 2
                                             : (coeff_bounds.size() - 2) / 2;
}
      
void
CurveCounter::
count(
    function<void(vector<int>&, vector<tuple<int,int>>&)> output,
    const vector<ReductionTable> & tables,
    const OpenCLInterface& opencl
    )
{
  auto poly_coeffs = vector<int>();
  poly_coeffs.reserve(this->degree() + 1);
  this->count_recursive(output, tables, opencl, poly_coeffs);
}

void
CurveCounter::
count_recursive(
    function<void(vector<int>&, vector<tuple<int,int>>&)> output,
    const vector<ReductionTable> & tables,
    const OpenCLInterface & opencl,
    vector<int>& poly_coeffs
    )
{
  size_t ix = poly_coeffs.size();

  if (ix > this->degree()) {
    auto curve = Curve(this->prime, poly_coeffs);
    if (curve.has_squarefree_rhs()) {
      auto nmb_points = curve.isogeny_nmb_points(tables, opencl);
      output(poly_coeffs, nmb_points);
    }
  }
  else {
    poly_coeffs.push_back(0);
    for (int c = get<0>(this->coeff_bounds[ix]); c < get<1>(this->coeff_bounds[ix]); ++c) {
      poly_coeffs[ix] = c;
      this->count_recursive(output, tables, opencl, poly_coeffs);
    }
    poly_coeffs.pop_back();
  }
}

CurveEnumerator::
CurveEnumerator(
    int prime,
    int genus,
    int package_size
    ) :
  prime( prime ), genus_( genus ), package_size( package_size ),
  is_end_( false )
{
  this->poly_coeffs = vector<int>(this->degree(), 0);
  this->poly_coeffs.back() = 1;
  this->dx = 0;
}


vector<tuple<int,int>>
CurveEnumerator::
as_bounds()
{
  auto coeff_bounds = vector<tuple<int,int>>();
  coeff_bounds.reserve(this->poly_coeffs.size());

  for (size_t dx=0; dx<this->poly_coeffs.size(); ++dx) {
    int lbd = this->poly_coeffs[dx];
    int ubd = dx != 0 ? lbd + 1
                      : (lbd + this->package_size <= this->prime ? lbd + this->package_size
                                                                 : this->prime);
    coeff_bounds.push_back(make_tuple(lbd,ubd));
  }

  return coeff_bounds;
}

CurveEnumerator &
CurveEnumerator::
step()
{
  if (this->is_end()) return *this;

  int dx = this->dx;
  if (dx == 0) {
    this->poly_coeffs[dx] += this->package_size;
    if (this->poly_coeffs[dx] >= this->prime) {
      this->poly_coeffs[dx] = 0;
      ++this->dx;
      return this->step();
    } else
      return *this;

  } else if (dx == this->poly_coeffs.size()) {
    if (this->poly_coeffs.size() == this->degree()) {
      this->poly_coeffs = vector<int>(this->degree()+1, 0);
      this->poly_coeffs.back() = 1;
      this->dx = 0;
      return this->step();
    } else {
      this->is_end_ = true;
      return *this;
    }

  } else if (dx == this->poly_coeffs.size() - 1) {
    if (this->prime != 2 && this->poly_coeffs.back() == 1) {
      this->poly_coeffs.back() = this->fp_non_square();
      fill(poly_coeffs.begin(), poly_coeffs.end()-1, 0);
      this->dx = 0;
      return this->step();
    } else {
      ++this->dx;
      return this->step();
    }

  } else if (dx == this->poly_coeffs.size() - 2) {
    ++this->dx;
    return this->step();

  } else {
    ++this->poly_coeffs[dx];
    if (this->poly_coeffs[dx] >= this->prime) {
      fill(poly_coeffs.begin(), poly_coeffs.begin()+dx, 0);
      ++this->dx;
      return this->step();
    } else {
      this->dx = 0;
      return *this;
    }
  }
}

int
CurveEnumerator::
fp_non_square()
{
  set<int> squares;
  for (int x=1; x<this->prime; ++x)
    squares.insert(x*x % this->prime);

  for (int x=1; x<this->prime; ++x)
    if (squares.find(x) == squares.end())
      return x;
}
