#ifndef _H_ENUMERATION_TABLE
#define _H_ENUMERATION_TABLE


using namespace std;


class EnumerationTable
{
  public:
    EnumerationTable( unsigned int prime, unsigned int prime_exponent );
    ~EnumerationTable();

    bool inline is_prime_field() const { return this->prime_exponent == 1; };

    const unsigned int inline operator[](int ix) const
    {
      return nmod_poly_get_coeff_ui(this->fq_elements[ix], 0);
    };
    const fq_nmod_t inline operator[](int ix) const { return this->fq_elements[ix]; };

    unsigned int inline zero_index() const { return this->prime_power_pred; };
    // todo: this should be unsigned int
    tuple<int,int> inline block_non_zero() const { return make_tuple(0, this->prime_power_pred); };
    tuple<int,int> inline block_complete() const { return make_tuple(0, this->prime_power); };
    vector<unsigned int> power_coset_representatives(unsigned int n);

    unsigned int inline reduce_index(unsigned int ix) { return ix % this->prime_power_pred; };

    friend class Curve;
    friend class CurveEnumerator;

  protected:
    const unsigned int prime;
    const unsigned int prime_exponent;
    unsigned int prime_power;
    unsigned int prime_power_pred;
    // todo: implement
    unsigned int primeinv_flint;

  private:
    fq_ctx_t fq_ctx;
    vector<fq_nmod_t> fq_elements;
};

#endif
