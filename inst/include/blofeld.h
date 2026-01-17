// [[Rcpp::plugins(cpp20)]]

#define R_NO_REMAP

#include <vector>
#include <array>
#include <cmath>
#include <format>
#include <memory>

#include "blofeld/groups/Compartment.h"
#include "blofeld/groups/SEIDRVMZgroup.h"

//#include "blofeld/utilities/BridgeCpp.h"
#include "blofeld/utilities/BridgeRcpp.h"
#include "blofeld/utilities/ContainerFormatter.h"

#include "blofeld/Rcpp_wrappers/GroupWrapper.h"
#include "blofeld/Rcpp_wrappers/rcpp_module_macros.h"
