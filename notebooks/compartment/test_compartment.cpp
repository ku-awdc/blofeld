// [[Rcpp::plugins(cpp20)]]
// [[Rcpp::depends(blofeld)]]

#include <Rcpp.h>
#include "../../inst/include/blofeld.h"
#include "../../inst/include/blofeld/rcpp_wrappers/compartment_wrapper.h"

constexpr struct
{
  bool const debug = true;
  double const tol = 0.00001;
  // using Bridge = blofeld::BridgeMT19937;
  using Bridge = blofeld::BridgeRcpp;
} cts;

constexpr auto cc = blofeld::compartment_info(1);

using CompType = blofeld::Compartment<cts, blofeld::ModelType::Deterministic, cc>;
using CompWrap = blofeld::CompartmentWrapper<CompType>;


RCPP_MODULE(comp_test){
  using namespace Rcpp;

  class_<CompWrap>("CompWrap")
    .constructor()
    .method("distribute", &CompWrap::distribute)
    .method("applyChanges", &CompWrap::applyChanges)
    .method("getValues", &CompWrap::getValues)
  ;
  
}

/*

    .method("update", &NAME::update) \
    .method("get_parameters", &NAME::get_parameters) \
    .method("set_parameters", &NAME::set_parameters) \
    .method("get_full_state", &NAME::get_full_state) \
    .method("get_state", &NAME::get_state) \
    .method("set_state", &NAME::set_state) \
    .property("external_infection", &NAME::get_external_infection,  &NAME::set_external_infection) \

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

RCPP_MODULE(blofeld_test){
  using namespace Rcpp;

  GROUP_CLASS(Group)
}
*/
