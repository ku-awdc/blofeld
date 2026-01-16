#ifndef BLOFELD_RCPP_MODULE_H
#define BLOFELD_RCPP_MODULE_H

// [[Rcpp::plugins(cpp20)]]

#include <Rcpp.h>

#include "GroupWrapper.h"

using BasicGroup = blofeld::BasicGroup;

RCPP_MODULE(blofeld_rcpp_base){
  using namespace Rcpp;
  class_<BasicGroup>("BasicGroup")
    .constructor()
    .method("update", &BasicGroup::update)
    .method("get_parameters", &BasicGroup::get_parameters)
    .method("set_parameters", &BasicGroup::set_parameters)
    .method("get_full_state", &BasicGroup::get_full_state)
    .method("get_state", &BasicGroup::get_state)
    .method("set_state", &BasicGroup::set_state)
    .property("external_infection", &BasicGroup::get_external_infection,  &BasicGroup::set_external_infection)
  ;
}

#endif // BLOFELD_RCPP_MODULE_H
