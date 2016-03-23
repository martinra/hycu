#include <set>

#include <block_enumerator.hh>


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
