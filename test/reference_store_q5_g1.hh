#ifndef _H_TEST_STORE_REFERENCE_Q5_G1
#define _H_TEST_STORE_REFERENCE_Q5_G1


template <>
TestStore<5, 1,
          HyCu::CurveData::ExplicitRamificationHasseWeil,
          HyCu::StoreData::Count>
create_reference_store<5, 1,
                       HyCu::CurveData::ExplicitRamificationHasseWeil,
                       HyCu::StoreData::Count>()
{
  map<typename HyCu::CurveData::ExplicitRamificationHasseWeil::ValueType,
      typename HyCu::StoreData::Count::ValueType>
    store;

  store[{ vector<unsigned int>{ 1,1,1,1 }, vector<int>{ -2 } }] = { 30 };
  store[{ vector<unsigned int>{ 1,1,1,1 }, vector<int>{ 2 } }] = { 30 };
  store[{ vector<unsigned int>{ 1,1,2 }, vector<int>{ -4 } }] = { 60 };
  store[{ vector<unsigned int>{ 1,1,2 }, vector<int>{ -2 } }] = { 120 };
  store[{ vector<unsigned int>{ 1,1,2 }, vector<int>{ 0 } }] = { 240 };
  store[{ vector<unsigned int>{ 1,1,2 }, vector<int>{ 2 } }] = { 120 };
  store[{ vector<unsigned int>{ 1,1,2 }, vector<int>{ 4 } }] = { 60 };
  store[{ vector<unsigned int>{ 1,3 }, vector<int>{ -3 } }] = { 240 };
  store[{ vector<unsigned int>{ 1,3 }, vector<int>{ -1 } }] = { 240 };
  store[{ vector<unsigned int>{ 1,3 }, vector<int>{ 1 } }] = { 240 };
  store[{ vector<unsigned int>{ 1,3 }, vector<int>{ 3 } }] = { 240 };
  store[{ vector<unsigned int>{ 2,2 }, vector<int>{ -2 } }] = { 90 };
  store[{ vector<unsigned int>{ 2,2 }, vector<int>{ 2 } }] = { 90 };
  store[{ vector<unsigned int>{ 4 }, vector<int>{ -4 } }] = { 60 };
  store[{ vector<unsigned int>{ 4 }, vector<int>{ -2 } }] = { 120 };
  store[{ vector<unsigned int>{ 4 }, vector<int>{ 0 } }] = { 240 };
  store[{ vector<unsigned int>{ 4 }, vector<int>{ 2 } }] = { 120 };
  store[{ vector<unsigned int>{ 4 }, vector<int>{ 4 } }] = { 60 };

  return TestStore<5, 1,
                   HyCu::CurveData::ExplicitRamificationHasseWeil,
                   HyCu::StoreData::Count>
             (store);
}

/*
template
TestStore<5, 1,
          HyCu::CurveData::ExplicitRamificationHasseWeil,
          HyCu::StoreData::Count>
create_reference_store<5,1,
                        HyCu::CurveData::ExplicitRamificationHasseWeil,
                        HyCu::StoreData::Count>();
*/

#endif
