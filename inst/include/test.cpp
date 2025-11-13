// clang -std=c++20 -Wall -pedantic -o test test.cpp

#include "blofeld/Compartment.h"

/*class Bridge
{
public:
  virtual void stop(std::string_view const msg) = 0;
  virtual void print(std::string_view const msg) = 0;
};
*/

class BridgeCpp //: Bridge
{
public:
  void stop(std::string_view const msg) //override
  {
    throw(msg);
  };
  void print(std::string_view const msg) //override
  {
    std::cout << msg;
  };
};


int main(int argc, char *argv[])
{
  
  BridgeCpp bridge;
  
  struct CompileTimeSettings
  {
    bool const debug = true;
    double const tol = 0.00001;
    typedef BridgeCpp Bridge;
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
   
  return 0;
}
