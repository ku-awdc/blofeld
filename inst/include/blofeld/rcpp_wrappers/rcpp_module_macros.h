#ifndef BLOFELD_RCPP_MODULE_MACROS_H
#define BLOFELD_RCPP_MODULE_MACROS_H

#define GROUP_CLASS(NAME) \
  class_<NAME>(#NAME) \
    .constructor() \
    .method("update", &NAME::update) \
    .method("get_parameters", &NAME::get_parameters) \
    .method("set_parameters", &NAME::set_parameters) \
    .method("get_full_state", &NAME::get_full_state) \
    .method("get_state", &NAME::get_state) \
    .method("set_state", &NAME::set_state) \
    .property("external_infection", &NAME::get_external_infection,  &NAME::set_external_infection) \
  ;
  
#endif // BLOFELD_RCPP_MODULE_MACROS_H
