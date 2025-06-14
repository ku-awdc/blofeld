#include <Rcpp.h>

#include <type_traits>

#include <array>
#include <vector>


enum class CT { fixed, max, free, zero, balance };

template <CT s_ct, typename t_type, size_t s_size = 0>
class CompStorage;

template <typename t_type>
class CompStorage<CT::fixed, t_type, 0> : public std::array<t_type, 0>
{
  // TODO: is this needed?
};

template <typename t_type, size_t s_size>
class CompStorage<CT::fixed, t_type, s_size> : public std::array<t_type, s_size>
{
  CompStorage() = delete;

public:
  CompStorage(int const size)
  {
    if (size != s_size) Rcpp::stop("Invalid size for fixed storage");
    std::array<t_type, s_size>::fill(static_cast<t_type>(0));
  }

};

template <typename t_type, size_t s_size>
class CompStorage<CT::max, t_type, s_size> : public std::array<t_type, s_size>
{
  size_t m_size = 0;

  CompStorage() = delete;

public:

  CompStorage(int const size)
  {
    if (size > s_size) Rcpp::stop("Invalid size for max storage");
    std::array<t_type, s_size>::fill(static_cast<t_type>(0));
    m_size = size;
  }

  void resize(size_t const size)
  {
    m_size = size;
  }

  size_t size() const
  {
    return m_size;
  }

  // TODO: end iterator operator

};

template <typename t_type, size_t s_size>
class CompStorage<CT::free, t_type, s_size> : public std::vector<t_type>
{

  CompStorage() = delete;

public:
  CompStorage(size_t const size)
  {
    static_assert(s_size == 0);
    std::vector<t_type>::resize(size);
  }

};

template <typename t_type, size_t s_size>
class CompStorage<CT::zero, t_type, s_size>
{

};

template <typename t_type, size_t s_size>
class CompStorage<CT::balance, t_type, s_size>
{

};


int tfun()
{
  {
    CompStorage<CT::free, double, 0> tt(5);
    Rcpp::Rcout << "Free len=" << tt.size() << ": ";
    for(auto const& val : tt){
      Rcpp::Rcout << val << ", ";
    }
    Rcpp::Rcout << "\n";
  }

  {
    CompStorage<CT::fixed, double, 6> tt(6);
    Rcpp::Rcout << "Fixed len=" << tt.size() << ": ";
    for(auto const& val : tt){
      Rcpp::Rcout << val << ", ";
    }
    Rcpp::Rcout << "\n";\
  }

  {
    CompStorage<CT::max, double, 10> tt(7);
    Rcpp::Rcout << "Max len=" << tt.size() << ": ";
    for(auto const& val : tt){
      Rcpp::Rcout << val << ", ";
    }
    Rcpp::Rcout << "\n";\
  }

  return 1;
}

int ttv = tfun();


template <CT s_ct, size_t s_size = 0>
class TestCompartment
{
  CompStorage<s_ct, double, s_size> m_storage {};

public:

};




template <bool s_tt>
class TestClass
{
public:

  bool m_status = s_tt;
  int m_val = 0;

  TestClass(int const len)
  {
    Rcpp::Rcout << s_tt << "\n";
    m_val = len;
  }

};

/*
template <CT s_ct, size_t s_size, typename t_type>
t_type factfun(int const len)
{
  CompStorage<s_ct, s_size> rv {};

  if constexpr (s_ct == CT::fixed){
    // t_type rv = TestClass<true>(len);
  } else if constexpr (s_ct == CT::free){
    // t_type rv = TestClass<false>(len);
    rv.resize(len);
  } else if constexpr (s_ct == CT::max){
    // t_type rv = TestClass<false>(len);
    rv.resize(len);
  } else {
    t_type rv = TestClass<true>(len);
    return rv;
  }

  Rcpp::Rcout << rv.size() << "\n";
  return rv;

}
*/
//auto test = factfun<CT::fixed, 5, CompStorage<CT::fixed, 5>>(10);
//auto test2 = factfun<CT::free, 0, CompStorage<CT::free, 0>>(10);
//auto test3 = factfun<CT::max, 20, CompStorage<CT::max, 20>>(8);
//auto test3 = factfun<CT::unborn, TestClass<true>>(10);

/*


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


RCPP_MODULE(compartment_example){

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

*/
