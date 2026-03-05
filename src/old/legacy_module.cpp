#define R_NO_REMAP
#include <Rcpp.h>
#define R_NO_REMAP

#include "../inst/legacy/blofeld.h"

int test()
{
  return 1;
}

constexpr struct
{
  bool const debug = true;
  double const tol = 0.00001;
  // using Bridge = blofeld::BridgeMT19937;
  using Bridge = blofeld::BridgeRcpp;
} cts;

using GroupType = blofeld::SEIDRVMZgroup<cts, blofeld::ModelType::deterministic,
  blofeld::component(1), // S
  blofeld::component(20), // E
  blofeld::component(0), // L
  blofeld::component(3), // I
  blofeld::component(0), // D
  blofeld::component(1), // R
  blofeld::component(0), // V
  blofeld::component(1), // M
  blofeld::component(1)  // Z
>;
using Group = blofeld::GroupWrapper<cts, GroupType>;

using Comp1 = blofeld::CompartmentWrapper<cts, blofeld::Compartment<cts, blofeld::ModelType::deterministic, blofeld::component(1)>>;
using Comp10 = blofeld::CompartmentWrapper<cts, blofeld::Compartment<cts, blofeld::ModelType::deterministic, blofeld::component(10)>>;

using Comp = Comp1;

RCPP_MODULE(blofeld_legacy_module){

	using namespace Rcpp;

  function("test", &test);
  
  class_<Comp>("Comp")
    .constructor()
    .property("sum", &Comp::get_sum, &Comp::set_sum)
    .property("values", &Comp::get_values, &Comp::set_values)
    .method("process", &Comp::process)
    .method("update", &Comp::update)
    .method("insert", &Comp::insert)
  ;
  
}
