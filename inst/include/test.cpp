// clang++ -std=c++20 -Wall -pedantic -o test test.cpp

#include "blofeld/Compartment.h"
#include "blofeld/utilities/BridgeCpp.h"


int main(int argc, char *argv[])
{
  
  blofeld::BridgeMT19937 bridge;
  
  struct CompileTimeSettings
  {
    bool const debug = true;
    double const tol = 0.00001;
    typedef blofeld::BridgeMT19937 Bridge;
  };
  
  constexpr CompileTimeSettings cts = {
    .debug = true
  };
  
  constexpr blofeld::ModelType mt = blofeld::ModelType::stochastic;
  constexpr blofeld::CompType ct = { 
    .compcont = blofeld::CompCont::array,
    .n = 10
  };
  
  blofeld::Compartment<cts, mt, ct> comp(bridge, 10);
  auto vv = comp.ptr();
  
  auto [carry, take] = comp.process_rate(0.4, std::array{ 0.1, 0.1, 0.1});
  std::cout << carry << ", " << take.size() << std::endl;
  
  return 0;
}
