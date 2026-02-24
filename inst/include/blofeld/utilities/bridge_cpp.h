#ifndef BLOFELD_BRIDGE_CPP_H
#define BLOFELD_BRIDGE_CPP_H

#include <random>
#include <format>
#include <iostream>
#include <stdexcept>

#include "./bridge.h"

namespace blofeld
{

  template<typename T_rng>
  class BridgeCpp : protected Bridge
  {
  private:
    // https://en.cppreference.com/w/cpp/numeric/random.html
    T_rng m_rng;

  public:
    // Generic constructor with no default for unknown RNG type
    explicit BridgeCpp(T_rng rng)
      : m_rng(std::move(rng))
    {
    }

    // Specialised constructor with default value for MT19937 initialisation
    explicit BridgeCpp(T_rng rng = [](){
      std::random_device r;
      std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
      T_rng rng(seed);
      return rng;
    }())
      requires(std::is_same_v<T_rng, std::mt19937>)
      : m_rng(std::move(rng))
    {
    }

    template<typename... Args>
    void print(std::format_string<Args...>&& fmt, Args&&... args)
    {
      Bridge::print(std::cout, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void println(std::format_string<Args...>&& fmt, Args&&... args)
    {
      print(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
      println();
    }

    void println()
    {
      std::cout << "\n";
    }

    template<typename... Args>
    void stop(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
      std::cerr << "ERROR: " << msg << std::endl;
      throw std::runtime_error(msg);
    }

    template<typename... Args>
    void warning(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
      std::cout << "WARNING: " << msg << "\n";
    }

    auto rbinom(int const n, double const p)
      -> int
    {
      // TODO: check n and p

      if(n==0) return(0);
      std::binomial_distribution<> d(n, p);
      return d(m_rng);
    }
    
    // Works with array or vector input rates (maybe also Rcpp::NumericVector ??):
    template <Container C>
    [[nodiscard]] auto rmultinom(int const total, C const& prob) noexcept(!Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<int>, std::array<int, C{}.size()>>
    {
      auto fun = [this](int n, double p) -> int { return rbinom(n,p); };
      // auto fun = std::bind(&BridgeCpp<T_rng>::rbinom, this, std::placeholders::_1, std::placeholders::_2);
      
      return Bridge::rmultinom(fun, total, prob);
    }


  };

  using BridgeMT19937 = BridgeCpp<std::mt19937>;

} //blofeld

#endif // BLOFELD_BRIDGE_CPP_H
