#include <Rcpp.h>

template <bool s_t>
class Simple
{
public:
  int i = 0;

  Simple(int ii, bool fake) : i(ii)
  {

  }

  Simple(Simple const& obj)
  {
    Rcpp::Rcout << "Copying...\n";
    i = obj.i+1;
  }

  // TODO: cloning can cause R to abort if the original pointer is invalid
  Simple clone() const
  {
    return Simple(*this);
  }

};
using SimpleTrue = Simple<true>;
RCPP_EXPOSED_AS(SimpleTrue)
RCPP_EXPOSED_WRAP(SimpleTrue)
// Note: can't use RCPP_EXPOSED_CLASS with aliases

int testfun()
{
  SimpleTrue tt = SimpleTrue(0, true);
  SimpleTrue copy = SimpleTrue(tt);
  return copy.i;
}


RCPP_MODULE(wrap_class_example){

	using namespace Rcpp;

  function("testfun", &testfun);

  class_<Simple<true>>("Simple")
    .factory(invalidate_default_constructor)
    .constructor<int, bool>("C'tor")
    .constructor<SimpleTrue&>("Copy c'tor")
    .field("i", &Simple<true>::i)
    .method("clone", &Simple<true>::clone)
  ;

}
