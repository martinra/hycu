
#ifndef _H_TEST_REFERENCE_STORE_HI_Q5_G1
#define _H_TEST_REFERENCE_STORE_HI_Q5_G1


template <>
TestStore<5, 1,
          HyCu::CurveData::HasseWeil,
          HyCu::StoreData::IsomorphismClass>
create_reference_store<5, 1,
                       HyCu::CurveData::HasseWeil,
                       HyCu::StoreData::IsomorphismClass>()
{
  map<typename HyCu::CurveData::HasseWeil::ValueType,
      typename HyCu::StoreData::IsomorphismClass::ValueType>
    store;


store[{ vector<int>{ -4 } }]
    = { vector<vector<int>>{
          { 0,1,1,4,0 },
          { 4,0,0,4,2 }
      } };
store[{ vector<int>{ -3 } }]
    = { vector<vector<int>>{
          { 0,0,0,4,3 }
      } };
store[{ vector<int>{ -2 } }]
    = { vector<vector<int>>{
          { 0,0,0,4,0 },
          { 0,0,4,2 },
          { 0,4,0,4,0 },
          { 0,4,4,4,2 },
          { 1,4,0,4,3 }
      } };
store[{ vector<int>{ -1 } }]
    = { vector<vector<int>>{
          { 0,0,0,4,2 }
      } };
store[{ vector<int>{ 0 } }]
    = { vector<vector<int>>{
          { 0,0,0,4,1 },
          { 0,1,1,4,3 },
          { 1,4,1,4,2 }
      } };
store[{ vector<int>{ 1 } }]
    = { vector<vector<int>>{
          { 0,0,4,4,3 }
      } };
store[{ vector<int>{ 2 } }]
    = { vector<vector<int>>{
          { 0,4,0,4,3 },
          { 0,4,4,4,0 },
          { 1,0,0,4,2 },
          { 1,4,4,4,3 },
          { 2,0,0,4,1 }
      } };
store[{ vector<int>{ 3 } }]
    = { vector<vector<int>>{
          { 0,1,1,4,1 }
      } };
store[{ vector<int>{ 4 } }]
    = { vector<vector<int>>{
          { 0,4,4,4,1 },
          { 4,0,4,1 }
      } };

  return TestStore<5, 1,
                   HyCu::CurveData::HasseWeil,
                   HyCu::StoreData::IsomorphismClass>
             (store);
}

template
TestStore<5, 1,
          HyCu::CurveData::HasseWeil,
          HyCu::StoreData::IsomorphismClass>
create_reference_store<5,1,
                        HyCu::CurveData::HasseWeil,
                        HyCu::StoreData::IsomorphismClass>();

#endif
