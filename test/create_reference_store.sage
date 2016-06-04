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

## h -4 classes given by program
## 0,4,4,4,3
## 1,4,4,4,2
## 3,0,0,4,3
## 4,0,0,4,2
## 4,0,4,3
## 4,1,4,2

def write_reference_store_hi(prime, genus):
  isogeny_isomorphism_classes = curves_isogeny_isomorphism_classes(prime, genus)

  code = """
#ifndef _H_TEST_REFERENCE_STORE_HI_Q{prime_power}_G{genus}
#define _H_TEST_REFERENCE_STORE_HI_Q{prime_power}_G{genus}


template <>
TestStore<{prime_power}, {genus},
          HyCu::CurveData::HasseWeil,
          HyCu::StoreData::IsomorphismClass>
create_reference_store<{prime_power}, {genus},
                       HyCu::CurveData::HasseWeil,
                       HyCu::StoreData::IsomorphismClass>()
{{
  map<typename HyCu::CurveData::HasseWeil::ValueType,
      typename HyCu::StoreData::IsomorphismClass::ValueType>
    store;

"""
    
  for (hasse_weil_offsets, isomorphism_classes) in \
        sorted(isogeny_isomorphism_classes.items()):

    isomorphism_classes_code = \
        "vector<vector<int>>{{\n" \
      + ",\n".join( "          {{ " + ",".join(str(c) for c in ic) + " }}"
                   for ic in isomorphism_classes ) \
      + "\n      }}"

    code += """
store[{{{{ vector<int>{{{{ {hasse_weil_offsets} }}}} }}}}]
    = {{{{ {isomorphism_classes_code} }}}};""" \
      .format( hasse_weil_offsets = ','.join(map(str, hasse_weil_offsets))
             , isomorphism_classes_code = isomorphism_classes_code
             )

  code += """

  return TestStore<{prime_power}, {genus},
                   HyCu::CurveData::HasseWeil,
                   HyCu::StoreData::IsomorphismClass>
             (store);
}}

template
TestStore<{prime_power}, {genus},
          HyCu::CurveData::HasseWeil,
          HyCu::StoreData::IsomorphismClass>
create_reference_store<{prime_power},{genus},
                        HyCu::CurveData::HasseWeil,
                        HyCu::StoreData::IsomorphismClass>();

#endif
"""

  with file("reference_store_hi_q{}_g{}.hh".format(prime,genus), 'w') as output:
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

def curves_isogeny_isomorphism_classes(prime, genus):
  isomorphism_classes = {}

  Ks = [GF(prime**n, 'a') for n in range(1,genus+1)]
  K = Ks[0]
  R.<x> = K[]
  Rz.<x,z> = K[]


  x_scale = lambda poly, e: poly.subs(x=K.multiplicative_generator()**e*x)
  x_shift = lambda poly, e: poly.subs(x=x+e*K.gen())
  y_scale = lambda poly, e: K.multiplicative_generator()**(2*e) * poly
  z_shift = lambda poly, e: poly.parent()( poly.homogenize(Rz.gen(1)).subs(z=1+e*K.gen()*x) )

  generator_power_dict = dict( (K.multiplicative_generator()**n,n) for n in range(prime-1) )
  generator_power_dict[K(0)] = prime-1

  for coeffs in itertools.chain( xmrange((2*genus+2)*[prime]),
                                 xmrange((2*genus+3)*[prime]) ):
    if coeffs[-1] == 0: continue
    poly = R(coeffs)

    try:
      (_,hasse_weil_offsets) = ramification_isogeny_rhs(poly, Ks)
    except ArithmeticError:
      continue


    tmp = [generator_power_dict[c] for c in poly.coefficients(sparse=False)]

    key = hasse_weil_offsets
    if key not in isomorphism_classes:
      isomorphism_classes[key] = [set([poly])]
    else:
      poly_transformations = set([
          x_scale(poly,1), x_scale(poly,-1)
        , x_shift(poly,1), x_shift(poly,-1)
        , y_scale(poly,1), y_scale(poly,-1)
        , z_shift(poly,1), z_shift(poly,-1)
        ])

      isomorphic_indices = []
      for (ix,ic) in enumerate(isomorphism_classes[key]):
        if not ic.isdisjoint(poly_transformations):
          isomorphic_indices.append(ix)
      if tmp == [ 0,1,1,4,2 ]:
        print poly
        print isomorphic_indices

      ic = set([poly])
      for ix in reversed(isomorphic_indices):
        ic = ic.union(isomorphism_classes[key][ix])
        isomorphism_classes[key] = \
          isomorphism_classes[key][:ix] + isomorphism_classes[key][ix+1:]
      isomorphism_classes[key].append(ic)


  for key in isomorphism_classes.keys():
    for ic in isomorphism_classes[key]:
      ic_gen_pw = sorted([ ([ generator_power_dict[c]
                             for c in p.coefficients(sparse=False) ], ix)
                           for (ix,p) in enumerate(isomorphism_classes[key][-1]) if is_reduced(p) ])
      if [ 0,1,1,4,2 ] in [p for (p,_) in ic_gen_pw]:
        print ic_gen_pw

    isomorphism_classes[key] = sorted(
      [ sorted([ [ generator_power_dict[c]
                   for c in p.coefficients(sparse=False) ]
                 for p in ic if is_reduced(p) ])[0]
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

def is_reduced(rhs):
  if rhs[rhs.degree()-1] != 0:
    return False

  K = rhs.base_ring()
  g = K.multiplicative_generator()
  support = sorted(rhs.exponents())
  if len(support) <= 2:
    return rhs[support[0]] in [g**0, g**1]
  else:
    return rhs[support[-2]] in [g**0, g**1] \
      and  rhs[support[-3]]/rhs[support[-2]] \
             in [g**n for n in range(support[-2]-support[-3])]
