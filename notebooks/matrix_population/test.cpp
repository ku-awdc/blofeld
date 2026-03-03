// [[Rcpp::plugins(cpp20)]]
// [[Rcpp::depends(blofeld)]]

#include <Rcpp.h>

// Legacy code:
#include "../../inst/legacy/blofeld/groups/SEIDRVMZgroup.h"
#include "../../inst/legacy/blofeld/utilities/BridgeRcpp.h"
#include "../../inst/legacy/blofeld/Rcpp_wrappers/rcpp_module_macros.h"
#include "../../inst/legacy/blofeld/Rcpp_wrappers/GroupWrapper.h"
#include "../../inst/legacy/blofeld/Rcpp_wrappers/matrix_population_wrapper.h"


constexpr struct
{
  bool const debug = true;
  double const tol = 0.00001;
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

using SIRGroup = blofeld::GroupWrapper<GroupType>;

RCPP_EXPOSED_AS(SIRGroup)
RCPP_EXPOSED_WRAP(SIRGroup)

using MPop = blofeld::MatrixPopulationWrapper<blofeld::MatrixPopulation<cts, GroupType>>;
RCPP_EXPOSED_AS(MPop)
RCPP_EXPOSED_WRAP(MPop)

RCPP_MODULE(blofeld_test){
  using namespace Rcpp;

  GROUP_CLASS(SIRGroup)

  class_<MPop>("MPop")
    //.constructor<Test&>("C'tor")
    .constructor<Rcpp::List>("C'tor")
    //.method("test", &MPop::test)
    .method("show", &MPop::show)
    .method("getGroup", &MPop::getGroup)
    .method("update", &MPop::update)
    .method("getState", &MPop::getState)
    .method("setBetaMatrix", &MPop::setBetaMatrix)
  ;
}

