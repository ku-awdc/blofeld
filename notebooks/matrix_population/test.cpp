/*
 * Instructions:  change numE, numI, and/or numR below as needed
 * Note: this compiles both deterministic and stochastic models
 * If you need more speed set debug=false
 */
constexpr int numE = 0;
constexpr int numI = 3;
constexpr int numR = 1;
constexpr bool debug = true;


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
  bool const debug = false;
  double const tol = 0.00001;
  using Bridge = blofeld::BridgeRcpp;
} cts {
  .debug = debug
};

using SG = blofeld::SEIDRVMZgroup<cts, blofeld::ModelType::stochastic,
  blofeld::component(1), // S
  blofeld::component(numE), // E
  blofeld::component(0), // L
  blofeld::component(numI), // I
  blofeld::component(0), // D
  blofeld::component(numR), // R
  blofeld::component(0), // V
  blofeld::component(1), // M
  blofeld::component(1)  // Z
  >;

using DG = blofeld::SEIDRVMZgroup<cts, blofeld::ModelType::deterministic,
  blofeld::component(1), // S
  blofeld::component(numE), // E
  blofeld::component(0), // L
  blofeld::component(numI), // I
  blofeld::component(0), // D
  blofeld::component(numR), // R
  blofeld::component(0), // V
  blofeld::component(1), // M
  blofeld::component(1)  // Z
  >;

using StochasticGroup = blofeld::GroupWrapper<SG>;
RCPP_EXPOSED_AS(StochasticGroup)
RCPP_EXPOSED_WRAP(StochasticGroup)

using DeterministicGroup = blofeld::GroupWrapper<DG>;
RCPP_EXPOSED_AS(DeterministicGroup)
RCPP_EXPOSED_WRAP(DeterministicGroup)

using StochasticPop = blofeld::MatrixPopulationWrapper<blofeld::MatrixPopulation<cts, SG>>;
RCPP_EXPOSED_AS(StochasticPop)
RCPP_EXPOSED_WRAP(StochasticPop)

using DeterministicPop = blofeld::MatrixPopulationWrapper<blofeld::MatrixPopulation<cts, DG>>;
RCPP_EXPOSED_AS(DeterministicPop)
RCPP_EXPOSED_WRAP(DeterministicPop)

RCPP_MODULE(blofeld_test){
  using namespace Rcpp;

  GROUP_CLASS(StochasticGroup)
  GROUP_CLASS(DeterministicGroup)

  class_<StochasticPop>("StochasticPop")
    .constructor<Rcpp::List>("C'tor")
    .method("show", &StochasticPop::show)
    //.method("getGroup", &StochasticPop::getGroup)
    .method("update", &StochasticPop::update)
    .method("getState", &StochasticPop::getState)
    .method("setBetaMatrix", &StochasticPop::setBetaMatrix)
  ;

  class_<DeterministicPop>("DeterministicPop")
    .constructor<Rcpp::List>("C'tor")
    .method("show", &DeterministicPop::show)
  //.method("getGroup", &DeterministicPop::getGroup)
    .method("update", &DeterministicPop::update)
    .method("getState", &DeterministicPop::getState)
    .method("setBetaMatrix", &DeterministicPop::setBetaMatrix)
  ;
}

