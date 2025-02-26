#include <Rcpp.h>

#include "blofeld.h"

/*
TODO

1. Use concepts??

3. Add Settings as a template to all other classes
4. Separate .h and .ipp files, but include both from blofeld.h
5. Use blofeld namespace
6. Create a base blofeld module in headers so that we can derive module interfaces via inherited classes
7. Use contracts within metapop and metaspread and metadetect to ensure they derive from pop/spread/detect
8. Have a static class used to track and number Info classes
9. Use s_ for template (and static) variables, and T_ for template types
*/

template<blofeld::Options s_options>
class TestBase
{
  int m_val;

public:
  TestBase(const int val)
    : m_val(val)
  {
    if constexpr (s_options.debug)
    {
//      Rcpp::Rcout << s_options.custom.test << "\n";
      Rprintf("DEBUG\n");
    }else{
      Rprintf("FAST\n");
    }

  }

  int test()
  {
    blofeld::Info info{};
    blofeld::Population<blofeld::Options{}> pop(info);
    pop.update();

    return m_val;
  }
};

struct OptionsTest
{
  bool test = true;
};


constexpr blofeld::Options<OptionsTest> options{ .debug = true, .log_level=10, .custom { .test=false } };
constexpr blofeld::Options<OptionsTest> options2{ .debug = true, .log_level=10 };
constexpr blofeld::Options options3{ .debug = false, .log_level=10 };
constexpr blofeld::Options options4;


RCPP_MODULE(blofeld_module){

	using namespace Rcpp;

  using Test = TestBase<options3>;
	class_<Test>("Test")
	  .constructor<int const>("Constructor")
	  .factory(invalidate_default_constructor)

		.method("test", &Test::test, "Infect method")
	;

}
