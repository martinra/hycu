def write_reference_store(prime, genus):
  curve_count = count_curves(prime, genus)

  code = \
    [ "#ifndef _H_TEST_STORE_REFERENCE_Q{prime_power}_G{genus}"
    , "#define _H_TEST_STORE_REFERENCE_Q{prime_power}_G{genus}"
    , ""
    , ""
    , "template <>"
    , "TestStore<{prime_power}, {genus},"
    , "          HyCu::CurveData::ExplicitRamificationHasseWeil,"
    , "          HyCu::StoreData::Count>"
    , "create_reference_store<{prime_power}, {genus},"
    , "                       HyCu::CurveData::ExplicitRamificationHasseWeil,"
    , "                       HyCu::StoreData::Count>()"
    , "{{"
    , "  map<typename HyCu::CurveData::ExplicitRamificationHasseWeil::ValueType,"
    , "      typename HyCu::StoreData::Count::ValueType>"
    , "    store;"
    , ""
    ]
    
  for ((ramification_type,hasse_weil_offsets), count) in sorted(curve_count.items()):
    code.append(
      "  store[{{{{ vector<int>{{{{ {ramification_type} }}}}, vector<int>{{{{ {hasse_weil_offsets} }}}} }}}}] = {{{{ {count} }}}};"
      .format( ramification_type = ','.join(map(str, ramification_type))
             , hasse_weil_offsets = ','.join(map(str, hasse_weil_offsets))
             , count = str(count)
             )
      )

  code += \
    [ ""
    , "  return TestStore<{prime_power}, {genus},"
    , "                   HyCu::CurveData::ExplicitRamificationHasseWeil,"
    , "                   HyCu::StoreData::Count>"
    , "             (store);"
    , "}}"
    , ""
    , "template"
    , "TestStore<{prime_power}, {genus},"
    , "          HyCu::CurveData::ExplicitRamificationHasseWeil,"
    , "          HyCu::StoreData::Count>"
    , "create_reference_store<{prime_power},{genus},"
    , "                        HyCu::CurveData::ExplicitRamificationHasseWeil,"
    , "                        HyCu::StoreData::Count>();"
    , ""
    , "#endif"
    ]

  with file("reference_store_q{}_g{}.hh".format(prime,genus), 'w') as output:
    output.writelines([line.format(prime_power = prime, genus = genus) + "\n" for line in code])


def count_curves(prime, genus):
  curve_count = {}

  Ks = [GF(prime**n, 'a') for n in range(1,genus+1)]
  R.<x> = GF(prime)[]

  for coeffs in xmrange((2*genus+2)*[prime]):
    if coeffs[-1] == 0: continue
    count_rhs(R(coeffs), Ks, curve_count)
  for coeffs in xmrange((2*genus+3)*[prime]):
    if coeffs[-1] == 0: continue
    count_rhs(R(coeffs), Ks, curve_count)

  return curve_count

def count_rhs(rhs, Ks, curve_count):
  if not rhs.is_squarefree(): return

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
  try:
    curve_count[(ramification_type,hasse_weil_offsets)] += 1
  except KeyError:
    curve_count[(ramification_type,hasse_weil_offsets)] = 1
