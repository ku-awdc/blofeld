#include <Rcpp.h>

namespace blofeld
{
  
  struct ClassRegister{
    int number = 0;
  };
  
}

static blofeld::ClassRegister class_register;

// [[Rcpp::interfaces(r, cpp)]]
int register_blofeld_class()
{
  class_register.number++;
  return class_register.number;
}

