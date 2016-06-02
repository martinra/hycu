import itertools


def write_reference_store_rhc(prime, genus):
  ramification_isogeny_count = curves_ramification_isogeny_count(prime, genus)

  code = """
#ifndef _H_TEST_REFERENCE_STORE_RHC_Q{prime_power}_G{genus}
#define _H_TEST_REFERENCE_STORE_RHC_Q{prime_power}_G{genus}


template <>
TestStore<{prime_power}, {genus},
          HyCu::CurveData::ExplicitRamificationHasseWeil,
          HyCu::StoreData::Count>
create_reference_store<{prime_power}, {genus},
                       HyCu::CurveData::ExplicitRamificationHasseWeil,
                       HyCu::StoreData::Count>()
{{
  map<typename HyCu::CurveData::ExplicitRamificationHasseWeil::ValueType,
      typename HyCu::StoreData::Count::ValueType>
    store;

"""
    
  for ((ramification_type,hasse_weil_offsets), count) in sorted(ramification_isogeny_count.items()):
    code += """
  store[{{{{ vector<int>{{{{ {ramification_type} }}}},
          vector<int>{{{{ {hasse_weil_offsets} }}}} }}}}]
    = {{{{ {count} }}}};""" \
      .format( ramification_type = ','.join(map(str, ramification_type))
             , hasse_weil_offsets = ','.join(map(str, hasse_weil_offsets))
             , count = str(count)
             )

  code += """

  return TestStore<{prime_power}, {genus},
                   HyCu::CurveData::ExplicitRamificationHasseWeil,
                   HyCu::StoreData::Count>
             (store);
}}

template
TestStore<{prime_power}, {genus},
          HyCu::CurveData::ExplicitRamificationHasseWeil,
          HyCu::StoreData::Count>
create_reference_store<{prime_power},{genus},
                        HyCu::CurveData::ExplicitRamificationHasseWeil,
                        HyCu::StoreData::Count>();

#endif
"""

  with file("reference_store_rhc_q{}_g{}.hh".format(prime,genus), 'w') as output:
    output.write(code.format(prime_power = prime, genus = genus))


def write_reference_store_dhi(prime, genus):
  discriminant_isogeny_isomorphism_classes = curves_discriminant_isogeny_isomorphism_classes(prime, genus)

  code = """
#ifndef _H_TEST_REFERENCE_STORE_DHI_Q{prime_power}_G{genus}
#define _H_TEST_REFERENCE_STORE_DHI_Q{prime_power}_G{genus}


template <>
TestStore<{prime_power}, {genus},
          HyCu::CurveData::DiscrimiantHasseWeil,
          HyCu::StoreData::IsomorphismClass>
create_reference_store<{prime_power}, {genus},
                       HyCu::CurveData::DiscrimiantHasseWeil,
                       HyCu::StoreData::IsomorphismClass>()
{{
  map<typename HyCu::CurveData::DiscrimiantHasseWeil::ValueType,
      typename HyCu::StoreData::Count::IsomorphismClass>
    store;

"""
    
  for ((discriminant,hasse_weil_offsets), isomorphism_classes) in \
        sorted(discriminant_isogeny_isomorphism_classes.items()):

    isomorphism_classes_code = \
        "vector<vector<int>>{{\n" \
      + ",\n".join( "          {{ " + ",".join(str(c) for c in ic) + " }}"
                   for ic in isomorphism_classes ) \
      + "\n      }}"

    code += """
  store[{{{{ {discriminant}, vector<int>{{{{ {hasse_weil_offsets} }}}} }}}}]
    = {{{{ {isomorphism_classes_code} }}}};""" \
      .format( discriminant = discriminant
             , hasse_weil_offsets = ','.join(map(str, hasse_weil_offsets))
             , isomorphism_classes_code = isomorphism_classes_code
             )

  code += """

  return TestStore<{prime_power}, {genus},
                   HyCu::CurveData::DiscrimiantHasseWeil,
                   HyCu::StoreData::IsomorphismClass>
             (store);
}}

template
TestStore<{prime_power}, {genus},
          HyCu::CurveData::DiscrimiantHasseWeil,
          HyCu::StoreData::IsomorphismClass>
create_reference_store<{prime_power},{genus},
                        HyCu::CurveData::DiscrimiantHasseWeil,
                        HyCu::StoreData::IsomorphismClass>();

#endif
"""

  with file("reference_store_dhi_q{}_g{}.hh".format(prime,genus), 'w') as output:
    output.write(code.format(prime_power = prime, genus = genus))

def curves_ramification_isogeny_count(prime, genus):
  curve_count = {}

  R.<x> = GF(prime)[]
  Ks = [GF(prime**n, 'a') for n in range(1,genus+1)]

  for coeffs in itertools.chain( xmrange((2*genus+2)*[prime]),
                                 xmrange((2*genus+3)*[prime]) ):
    if coeffs[-1] == 0: continue

    try:
      key = ramification_isogeny_rhs(R(coeffs), Ks)
    except ArithmeticError:
      continue
    
    try:
      curve_count[key] += 1
    except KeyError:
      curve_count[key] = 1

  return curve_count

def curves_discriminant_isogeny_isomorphism_classes(prime, genus):
  isomorphism_classes = {}

  Ks = [GF(prime**n, 'a') for n in range(1,genus+1)]
  K = Ks[0]
  R.<x> = K[]
  Rz.<x,z> = K[]


  x_scale = lambda poly: poly.subs(x=K.multiplicative_generator()*x)
  x_shift = lambda poly: poly.subs(x=x+K.gen())
  y_scale = lambda poly: K.multiplicative_generator()**2 * poly
  z_shift = lambda poly: poly.parent()( poly.homogenize(Rz.gen(1)).subs(z=1+K.gen()) )

  for coeffs in itertools.chain( xmrange((2*genus+2)*[prime]),
                                 xmrange((2*genus+3)*[prime]) ):
    if coeffs[-1] == 0: continue
    poly = R(coeffs)

    try:
      (_,hasse_weil_offsets) = ramification_isogeny_rhs(poly, Ks)
    except ArithmeticError:
      continue

    key = (poly.discriminant(),hasse_weil_offsets)
    if key not in isomorphism_classes:
      isomorphism_classes[key] = [set([poly])]
    else:
      x_scaled = x_scale(poly)
      x_shifted = z_shift(poly)
      y_scaled = y_scale(poly)
      z_shifted = z_shift(poly)

      isomorphic_indices = []
      for (ix,ic) in enumerate(isomorphism_classes[key]):
        if (    x_scaled in ic or x_shifted in ic
             or y_scaled in ic or z_shifted in ic ):
          isomorphic_indices.append(ix)

      ic = set([poly])
      for ix in reversed(isomorphic_indices):
        ic = ic.union(isomorphism_classes[key][ix])
        isomorphism_classes[key] = \
          isomorphism_classes[key][:ix] + isomorphism_classes[key][ix+1:]
      isomorphism_classes[key].append(ic)


  generator_power_dict = dict( (K.multiplicative_generator()**n,n) for n in range(prime-1) )
  generator_power_dict[0] = prime-1

  for key in isomorphism_classes.keys():
    isomorphism_classes[key] = sorted(
      [ sorted([ [ generator_power_dict[c]
                   for c in p.coefficients(sparse=False) ]
                 for p in ic ])[0]
        for ic in isomorphism_classes[key] ])

  return isomorphism_classes

def ramification_isogeny_rhs(rhs, Ks):
  if not rhs.is_squarefree():
    raise ArithmeticError()

  hasse_weil_offsets = [0 for _ in range(len(Ks))]
  for (n,K) in enumerate(Ks):
    unramified = 0; ramified = 0
    for y in [rhs(x) for x in K]:
      if y == 0: ramified += 1
      elif y.is_square(): unramified += 2
    if rhs.degree() % 2 != 0: ramified += 1
    elif K(rhs.leading_coefficient()).is_square():
      unramified += 2

    hasse_weil_offsets[n] = len(K) + 1 - (unramified + ramified)

  ramification_type = sorted([f.degree() for (f,_) in list(rhs.factor())])
  if rhs.degree() % 2 == 1:
    ramification_type.insert(0,1)

  ramification_type = tuple(ramification_type)
  hasse_weil_offsets = tuple(hasse_weil_offsets)

  return (ramification_type,hasse_weil_offsets)
