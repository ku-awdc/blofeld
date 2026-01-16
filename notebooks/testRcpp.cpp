// [[Rcpp::plugins(cpp20)]]
// [[Rcpp::depends(blofeld)]]

#include <Rcpp.h>
#include <blofeld.h>

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

RCPP_MODULE(blofeld_test){
  using namespace Rcpp;
  class_<Group>("Group")
    .constructor()
    .method("update", &Group::update)
    .method("get_parameters", &Group::get_parameters)
    .method("set_parameters", &Group::set_parameters)
    .method("get_full_state", &Group::get_full_state)
    .method("get_state", &Group::get_state)
    .method("set_state", &Group::set_state)
    .property("external_infection", &Group::get_external_infection,  &Group::set_external_infection)
  ;
}
