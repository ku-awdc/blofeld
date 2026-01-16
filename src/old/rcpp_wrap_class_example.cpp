#include <Rcpp.h>

#include <type_traits>

enum class UpdateType { Deterministic, Stochastic };

consteval auto absval(int const size) ->
  size_t
{
  int const asize = size >= 0 ? size : -size;
  return static_cast<size_t>(asize);
}

template <int s_size, UpdateType s_type>
class Comp
{
public:

  static constexpr size_t s_ss = absval(s_size);
  size_t m_size = 0;

  typedef std::conditional<
    s_type == UpdateType::Deterministic,
    double,
    int
  >::type t_type;

  typedef std::conditional<
    s_size == -1,
    std::vector<t_type>,
    std::array<t_type, absval(s_size)>
  >::type t_container;

  // TODO: delegate the container to a wrapper class so we don't
  // mistakenly use m_container.size() - and also use ranges
  t_container m_container;

  Comp() = delete;

  Comp(size_t size)
  {
    if constexpr (s_type==UpdateType::Deterministic)
    {
      Rcpp::Rcout << "Deterministic\n" ;
    } else if constexpr (s_type==UpdateType::Stochastic)
    {
      Rcpp::Rcout << "Stochastic\n" ;
    } else {
      Rcpp::stop("Unrecognised UpdateType");
    }

    if (size < 0) Rcpp::stop("Negative size");
    m_size = size;

    if constexpr (s_size==-1)
    {
      m_container.resize(size);
    } else if constexpr (s_size < -1)
    {
      if ( m_size > s_size ) Rcpp::stop("Provided size is too big");
    } else {
      if ( m_size != s_size ) Rcpp::stop("Non-matching size");
    }
  }
};


Comp<0, UpdateType::Deterministic> cc1(0);
Comp<-1, UpdateType::Deterministic> cc2(10);
Comp<-2, UpdateType::Deterministic> cc3(1);
Comp<2, UpdateType::Stochastic> cc4(2);

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
    //.factory(invalidate_default_constructor)
    .constructor<int, bool>("C'tor")
    .constructor<SimpleTrue&>("Copy c'tor")
    .field("i", &Simple<true>::i)
    .method("clone", &Simple<true>::clone)
  ;

}
