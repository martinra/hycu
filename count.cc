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

#include <count.hh>

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

vector<int>
Curve::
poly_coefficients_as_powers(
    const ReductionTableFq & table
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
    const ReductionTableFq & table,
    const OpenCLInterface & opencl
  )
{
  // todo: Use error checking for all calls. Some implementations don't seem to support try/catch, but doublecheck this.
  int poly_coeffs_size = (int)this->poly_coeffs.size();
  vector<int> poly_coeffs_exponents = this->poly_coefficients_as_powers(table);
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
    const vector<ReductionTableFq>& tables,
    const OpenCLInterface& opencl
    )
{
  auto nmb_points = vector<tuple<int,int>>();
  nmb_points.reserve(tables.size());

  for (const ReductionTableFq & table : tables)
    nmb_points.push_back(this->count(table, opencl));

  return nmb_points;
}

ReductionTableFq::
ReductionTableFq(
    int prime,
    int prime_exponent,
    OpenCLInterface & opencl
    ) :
  prime( prime ),
  prime_exponent( prime_exponent ),
  prime_power( pow(prime,prime_exponent) )
{
  this->exponent_reduction_table = this->compute_exponent_reduction_table(prime_power);
  tie(this->incrementation_table, this->fp_exponents) =
      this->compute_incrementation_fp_exponents_tables(prime, prime_exponent, prime_power);

  this->buffer_exponent_reduction_table = make_shared<cl::Buffer>(
      *opencl.context, CL_MEM_READ_ONLY, sizeof(int) * this->exponent_reduction_table->size() );
  this->buffer_incrementation_table = make_shared<cl::Buffer>(
      *opencl.context, CL_MEM_READ_ONLY, sizeof(int) * this->incrementation_table->size() );

  opencl.queue->enqueueWriteBuffer(*this->buffer_exponent_reduction_table, CL_TRUE, 0,
      sizeof(int)*this->exponent_reduction_table->size(),
      this->exponent_reduction_table->data() );
  opencl.queue->enqueueWriteBuffer(*this->buffer_incrementation_table, CL_TRUE, 0,
      sizeof(int)*this->incrementation_table->size(),
      this->incrementation_table->data() );
}

shared_ptr<vector<int>>
ReductionTableFq::
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

tuple<shared_ptr<vector<int>>, shared_ptr<vector<int>>>
ReductionTableFq::
compute_incrementation_fp_exponents_tables(
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

  auto fp_exponents = make_shared<vector<int>>(prime);
  fp_exponents->at(0) = prime_power - 1; // special index for 0

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
    if (coeff_sum < prime)
      fp_exponents->at(coeff_sum) = ix;

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

  return make_tuple(incrementations, fp_exponents);
}

const std::string opencl_kernel_evaluation =
  "void\n"
  "kernel\n"
  "evaluate(\n"
  "  const int poly_size,\n"
  "  global const int * poly_coeffs_expontents,\n"
  "  const int nmb_units, // this is prime_power - 1\n"
  "  global const int * exponent_reduction_table,\n"
  "  global const int * incrementation_table,\n"
  "  \n"
  "  global int * nmbs_unramified,\n"
  "  global int * nmbs_ramified\n"
  "  )\n"
  "{\n"
  "  // The variable x = a^i is represented by i < nmb_units.\n"
  "  // The case x=0 will not occur, but any element 0 is represented by then number nmb_units.\n"
  "  int x = get_global_id(0);\n"
  "  \n"
  "  int f = poly_coeffs_expontents[0];\n"
  "  for (int dx=1, xpw=x; dx < poly_size; ++dx, xpw+=x) {\n"
  "    xpw = exponent_reduction_table[xpw];\n"
  "    if (poly_coeffs_expontents[dx] != nmb_units) { // i.e. coefficient is not zero\n"
  "      if (f == nmb_units) { // i.e. f = 0\n"
  "        f = poly_coeffs_expontents[dx] + xpw;\n"
  "        f = exponent_reduction_table[f];\n"
  "      } else {\n"
  "        int tmp = exponent_reduction_table[poly_coeffs_expontents[dx] + xpw];\n"
  "        \n"
  "        int tmp2;\n"
  "        if (tmp <= f) {\n"
  "          // this can be removed by doubling the size of incrementation_table\n"
  "          // and checking at nmb_units + tmp - f\n"
  "          tmp2 = f;\n"
  "          f = tmp;\n"
  "          tmp = tmp2;\n"
  "        }\n"
  "        tmp2 = incrementation_table[tmp-f];\n"
  "        if (tmp2 != nmb_units) {\n"
  "          f = f + tmp2;\n"
  "          f = exponent_reduction_table[f];\n"
  "        } else\n"
  "          f = nmb_units;\n"
  "      }\n"
  "    }\n"
  "  }\n"
  "  \n"
  "  if (f == nmb_units) {\n"
  "    nmbs_unramified[x] = 0;\n"
  "    nmbs_ramified[x] = 1;\n"
  "  } else if (f & 1) {\n"
  "    nmbs_unramified[x] = 0;\n"
  "    nmbs_ramified[x] = 0;\n"
  "  } else {\n"
  "    nmbs_unramified[x] = 2;\n"
  "    nmbs_ramified[x] = 0;\n"
  "  }\n"
  "}\n"
  ;

const std::string opencl_kernel_reduction =
  "void\n"
  "kernel\n"
  "reduction(\n"
  "  global int* nmbs_unramified,\n"
  "  global int* nmbs_ramified,\n"
  "  local int* scratch_unramified,\n"
  "  local int* scratch_ramified,\n"
  "  const int len,\n"
  "  global int* sum_nmbs_unramified,\n"
  "  global int* sum_nmbs_ramified\n"
  "  )\n"
  "{\n"
  "  int gsz = get_global_size(0);\n"
  "  int gix = get_global_id(0);\n"
  "  int lsz = get_local_size(0);\n"
  "  int lix = get_local_id(0);\n"
  "\n"
  "  int acc_unramified = 0;\n"
  "  int acc_ramified = 0;\n"
  "  for (int ix=get_global_id(0); ix<len; ix+=gsz) {\n"
  "    acc_unramified += nmbs_unramified[ix];\n"
  "    acc_ramified += nmbs_ramified[ix];\n"
  "  }\n"
  "\n"
  "  scratch_unramified[lix] = acc_unramified;\n"
  "  scratch_ramified[lix] = acc_ramified;\n"
  "\n"
  "  barrier(CLK_LOCAL_MEM_FENCE);\n"
  "\n"
  "  for (int offset = lsz/2; offset>0; offset/=2) {\n"
  "    if (lix < offset) {\n"
  "      scratch_unramified[lix] += scratch_unramified[lix+offset];\n"
  "      scratch_ramified[lix] += scratch_ramified[lix+offset];\n"
  "    }\n"
  "    barrier(CLK_LOCAL_MEM_FENCE);\n"
  "  }\n"
  "\n"
  "  if (lix == 0) {\n"
  "    sum_nmbs_unramified[get_group_id(0)] = scratch_unramified[0];\n"
  "    sum_nmbs_ramified[get_group_id(0)] = scratch_ramified[0];\n"
  "  }\n"
  "}\n"
  ;

OpenCLInterface::
OpenCLInterface()
{
  vector<cl::Platform> all_platforms;
  cl::Platform::get(&all_platforms);
  if (all_platforms.size() == 0)
    throw "No platforms found.";
  this->platform = make_shared<cl::Platform>(all_platforms[0]);

  vector<cl::Device> all_devices;
  this->platform->getDevices(CL_DEVICE_TYPE_GPU, &all_devices);
  if (all_devices.size() == 0)
    throw "No devices found.";
  this->device = make_shared<cl::Device>(all_devices[0]);

  this->context = make_shared<cl::Context>(*this->device);
  this->queue = make_shared<cl::CommandQueue>(*this->context, *this->device);

  cl::Program::Sources source_evaluation;
  source_evaluation.push_back({opencl_kernel_evaluation.c_str(), opencl_kernel_evaluation.length()});
  this->program_evaluation = make_shared<cl::Program>(*this->context, source_evaluation);
  if (this->program_evaluation->build({*this->device}) != CL_SUCCESS) {
    cerr << "Error building evaluation code:" << endl;
    cerr << this->program_evaluation->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*this->device);
    throw;
  }

  cl::Program::Sources source_reduction;
  source_reduction.push_back({opencl_kernel_reduction.c_str(), opencl_kernel_reduction.length()});
  this->program_reduction = make_shared<cl::Program>(*this->context, source_reduction);
  if (this->program_reduction->build({*this->device}) != CL_SUCCESS) {
    cerr << "Error building reduction code:" << endl;
    cerr << this->program_reduction->getBuildInfo<CL_PROGRAM_BUILD_LOG>(*this->device);
    throw;
  }
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
    const vector<ReductionTableFq> & tables,
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
    const vector<ReductionTableFq> & tables,
    const OpenCLInterface & opencl,
    vector<int>& poly_coeffs
    )
{
  size_t ix = poly_coeffs.size();

  if (ix > this->degree()) {
    auto nmb_points = Curve(this->prime, poly_coeffs).isogeny_nmb_points(tables, opencl);
    output(poly_coeffs, nmb_points);
  } else {
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
