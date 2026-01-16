#include <Rcpp.h>

#include "blofeld.h"

int test()
{
  return 1;
}

RCPP_MODULE(blofeld_module){

	using namespace Rcpp;

  function("test", &test);
}
