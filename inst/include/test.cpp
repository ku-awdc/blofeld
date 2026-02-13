// clang++ -std=c++20 -Wall -pedantic -o test test.cpp
// gcc-15 -std=c++20 -stdlib=libstdc++ -Wall -pedantic /opt/homebrew/lib/gcc/current/libstdc++.a -o test test.cpp

// TODO: sort out include order

//#include "blofeld/groups/compartment_types.h"
#include "blofeld/groups/container.h"

//#include "blofeld/utilities/bridge.h"
#include "blofeld/utilities/bridge_cpp.h"
#include "blofeld/utilities/container_formatter.h"

//#include "blofeld/groups/compartment.h"

#include <concepts>


int main ()
{
  
  constexpr auto ci = blofeld::compartment_info(1, blofeld::ContainerType::BirthDeath);
  auto ctr = blofeld::internal::Container<double, ci.container_type, ci.n>();
//  ctr.validate();
//  ctr.resize(2);
  
  struct CompileTimeSettings
  {
    bool const debug = true;
    double const tol = 0.00001;
    using Bridge = blofeld::BridgeMT19937;
  };
  
  using Bridge = CompileTimeSettings::Bridge;
  Bridge bridge;
  
  std::array arr {1, 2, 3};
  std::vector vec {4, 5, 6};
  
  //if constexpr (isFixedSize<decltype(arr)>)
  if constexpr (blofeld::Container<decltype(arr)> && blofeld::Fixedsize<decltype(arr)>)
  {
    bridge.println( "is fixed size: {}", arr);
  }

  if constexpr (blofeld::Container<decltype(arr)> && blofeld::Resizeable<decltype(vec)>)
  {
    bridge.println( "is resizeable: {}", vec);
  }
  
  bridge.println( "A container: {}", ctr );
  
  bridge.println( "A vector: {}", std::vector{1} );
  bridge.println( "An array: {}", std::array<int,0>{} );
  
  
  return 0;
}

/*
#include "blofeld/groups/Compartment.h"
#include "blofeld/groups/SEIDRVMZgroup.h"

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
    .n = 2
  };  
  
  blofeld::Compartment<cts, mt, ct> comp(bridge);
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
  
  constexpr blofeld::ModelType mt2 = blofeld::ModelType::stochastic;
  
  using GroupType = blofeld::SEIDRVMZgroup<cts, mt2,
    ct, // S
    ct, // E
    ct, // L
    ct, // I
    ct, // D
    ct, // R
    ct, // V
    ct, // M/C
    ct  // Z
  >;
    
  GroupType group(bridge);  
  
  for(int i=0; i<10000; ++i)
  {
    group.update();
  }
  
  return 0;
}

*/