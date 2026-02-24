#ifndef _BLOFELD_UTILITIES_H_
#define _BLOFELD_UTILITIES_H_

#include <Rcpp.h>

template <class RcppModuleClassName>
RcppModuleClassName* invalidate_default_constructor() {
  Rcpp::stop("Default constructor is disabled for this class");
  return 0;
}

#endif //_BLOFELD_UTILITIES_H_
