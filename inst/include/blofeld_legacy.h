// [[Rcpp::plugins(cpp20)]]

#define R_NO_REMAP

#include <vector>
#include <array>
#include <cmath>
#include <format>
#include <memory>

#include "../legacy/blofeld/groups/SEIDRVMZgroup.h"
#include "../legacy/blofeld/utilities/BridgeRcpp.h"
#include "../legacy/blofeld/Rcpp_wrappers/rcpp_module_macros.h"
#include "../legacy/blofeld/Rcpp_wrappers/GroupWrapper.h"
#include "../legacy/blofeld/Rcpp_wrappers/matrix_population_wrapper.h"
