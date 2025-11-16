// clang++ -std=c++20 -Wall -pedantic -o test test.cpp

#include "blofeld/Compartment.h"
#include "blofeld/utilities/BridgeCpp.h"
#include "blofeld/utilities/ContainerFormatter.h"

int main(int argc, char *argv[])
{

  struct CompileTimeSettings
  {
    bool const debug = true;
    double const tol = 0.00001;
    using Bridge = blofeld::BridgeMT19937;
  };
  
  constexpr CompileTimeSettings cts = {
    .debug = true
  };
  
  using Bridge = CompileTimeSettings::Bridge;
  Bridge bridge;
  
  double const bn = bridge.rbinom(10, 0.5);
  bridge.println("Binom: {}", bn);
  
  std::array mn = bridge.rmultinom(100000, std::array{0.1, 0.2, 0.3});
  bridge.println("Some stuff; {}; others", mn);

  bridge.print( "hi{}", 2 );
  bridge.println();
  bridge.println( "A vector: {}", std::vector{1} );
  bridge.println( "An array: {}", std::array<int,0>{} );
  
  constexpr blofeld::ModelType mt = blofeld::ModelType::stochastic;
  constexpr blofeld::CompType ct = { 
    .compcont = blofeld::CompCont::array,
    .n = 3
  };
  
  
  blofeld::Compartment<cts, mt, ct> comp(bridge, 3);
  int ss = 100;
  comp.set_sum(ss);
  bridge.println("{}", comp.ptr());
  bridge.println("{}", comp);
  
  int carried = 0;
  int taken = 0;
  for(int i=0; i<20; ++i)
  {
    auto [carry, take] = comp.process_rate(0.1, std::array{ 0.01, 0.02});
    carried += carry;
    taken += std::accumulate(take.begin(), take.end(), 0);
    comp.apply_changes();
    if(comp.get_sum()+carried+taken != ss)
    {
      bridge.println("Iter: {} - {},{}", comp, carried, taken);
      throw("cock");
    }
  }
  bridge.println("{} + {}", comp.get_sum(), carried+taken);
  
  return 0;
}
