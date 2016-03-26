#ifndef _H_FQ_ELEMENT_TABLE
#define _H_FQ_ELEMENT_TABLE

#include <tuple>
#include <vector>
#include <flint/fq_nmod.h>


using namespace std;


class Curve;


class FqElementTable
{
  public:
    FqElementTable( unsigned int prime, unsigned int prime_exponent );
    ~FqElementTable();

    bool inline is_prime_field() const { return this->prime_exponent == 1; };

    inline const unsigned int at_nmod(int ix) const
    {
      return nmod_poly_get_coeff_ui(this->fq_elements.at(ix), 0);
    };
    inline const fq_nmod_struct* at(int ix) const { return this->fq_elements[ix]; };
    inline const fq_nmod_struct* operator[](int ix) const { return this->fq_elements[ix]; };

    int inline zero_index() const { return this->prime_power_pred; };
    tuple<int,int> inline block_non_zero() const { return make_tuple(0, this->prime_power_pred); };
    tuple<int,int> inline block_complete() const { return make_tuple(0, this->prime_power); };
    vector<int> power_coset_representatives(unsigned int n) const;

    unsigned int inline reduce_index(unsigned int ix) const { return ix % this->prime_power_pred; };

    friend Curve;
    friend class CurveIterator;
    friend class IsogenyCountStore;
    friend class IsogenyRepresentativeStore;
    friend ostream& operator<<(ostream & stream, const Curve & curve);

  protected:
    const unsigned int prime;
    const unsigned int prime_exponent;
    unsigned int prime_power;
    unsigned int prime_power_pred;
    // todo: implement
    unsigned int primeinv_flint;

  private:
    fq_nmod_ctx_t fq_ctx;
    vector<fq_nmod_struct*> fq_elements;
};

#endif
