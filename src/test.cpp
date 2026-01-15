#include <Rcpp.h>

#include "blofeld.h"

struct CompileTimeSettings
{
  bool const debug = true;
  double const tol = 0.00001;
  using Bridge = blofeld::BridgeRcpp;
};

constexpr CompileTimeSettings cts = {
  .debug = true
};

// [[Rcpp::export]]
int testbridge()
{

  using Bridge = CompileTimeSettings::Bridge;
  Bridge bridge;

  std::array mn = bridge.rmultinom(100000, std::array{0.1, 0.2, 0.3});
  bridge.println("Some stuff; {}; others", mn);
  
  return 1;
}

/*
blofeld::Info info2{};
blofeld::Population<blofeld::Options{}> pop2(info2);
*/

/*
constexpr blofeld::Settings settings{ .debug = false, .log_level=10 };

consteval blofeld::Settings const& testing()
{
  return settings;
}
*/
