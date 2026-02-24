// [[Rcpp::plugins(cpp20)]]

#define R_NO_REMAP

#include <vector>
#include <array>
#include <cmath>
#include <format>
#include <memory>

#include "blofeld/groups/SEIDRVMZgroup.h"

#include "blofeld/utilities/bridge_rcpp.h"
#include "blofeld/utilities/container_formatter.h"

#include "blofeld/Rcpp_wrappers/compartment_wrapper.h"
#include "blofeld/Rcpp_wrappers/group_wrapper.h"
#include "blofeld/Rcpp_wrappers/rcpp_module_macros.h"
