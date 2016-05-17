#ifndef _H_TEST_STORE_REFERENCE_Q7_G1
#define _H_TEST_STORE_REFERENCE_Q7_G1


template <>
TestStore<7, 1,
          HyCu::CurveData::ExplicitRamificationHasseWeil,
          HyCu::StoreData::Count>
create_reference_store<7, 1,
                       HyCu::CurveData::ExplicitRamificationHasseWeil,
                       HyCu::StoreData::Count>()
{
  map<typename HyCu::CurveData::ExplicitRamificationHasseWeil::ValueType,
      typename HyCu::StoreData::Count::ValueType>
    store;

  store[{ vector<int>{ 1,1,1,1 }, vector<int>{ -4 } }] = { 84 };
  store[{ vector<int>{ 1,1,1,1 }, vector<int>{ 0 } }] = { 252 };
  store[{ vector<int>{ 1,1,1,1 }, vector<int>{ 4 } }] = { 84 };
  store[{ vector<int>{ 1,1,2 }, vector<int>{ -4 } }] = { 504 };
  store[{ vector<int>{ 1,1,2 }, vector<int>{ -2 } }] = { 1008 };
  store[{ vector<int>{ 1,1,2 }, vector<int>{ 0 } }] = { 504 };
  store[{ vector<int>{ 1,1,2 }, vector<int>{ 2 } }] = { 1008 };
  store[{ vector<int>{ 1,1,2 }, vector<int>{ 4 } }] = { 504 };
  store[{ vector<int>{ 1,3 }, vector<int>{ -5 } }] = { 336 };
  store[{ vector<int>{ 1,3 }, vector<int>{ -3 } }] = { 1008 };
  store[{ vector<int>{ 1,3 }, vector<int>{ -1 } }] = { 1344 };
  store[{ vector<int>{ 1,3 }, vector<int>{ 1 } }] = { 1344 };
  store[{ vector<int>{ 1,3 }, vector<int>{ 3 } }] = { 1008 };
  store[{ vector<int>{ 1,3 }, vector<int>{ 5 } }] = { 336 };
  store[{ vector<int>{ 2,2 }, vector<int>{ -4 } }] = { 252 };
  store[{ vector<int>{ 2,2 }, vector<int>{ 0 } }] = { 756 };
  store[{ vector<int>{ 2,2 }, vector<int>{ 4 } }] = { 252 };
  store[{ vector<int>{ 4 }, vector<int>{ -4 } }] = { 504 };
  store[{ vector<int>{ 4 }, vector<int>{ -2 } }] = { 1008 };
  store[{ vector<int>{ 4 }, vector<int>{ 0 } }] = { 504 };
  store[{ vector<int>{ 4 }, vector<int>{ 2 } }] = { 1008 };
  store[{ vector<int>{ 4 }, vector<int>{ 4 } }] = { 504 };

  return TestStore<7, 1,
                   HyCu::CurveData::ExplicitRamificationHasseWeil,
                   HyCu::StoreData::Count>
             (store);
}

template
TestStore<7, 1,
          HyCu::CurveData::ExplicitRamificationHasseWeil,
          HyCu::StoreData::Count>
create_reference_store<7,1,
                        HyCu::CurveData::ExplicitRamificationHasseWeil,
                        HyCu::StoreData::Count>();

#endif
