#ifndef BLOFELD_BRIDGE_RCPP_H
#define BLOFELD_BRIDGE_RCPP_H

#include <format>
#include <iostream>

#include <Rcpp.h>
#define R_NO_REMAP

#include "./bridge.h"

namespace blofeld
{

  class BridgeRcpp : protected Bridge
  {
  private:

  public:
    explicit BridgeRcpp()
    {

    }

    template<typename... Args>
    void print(std::format_string<Args...>&& fmt, Args&&... args)
    {
      Bridge::print(Rcpp::Rcout, std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void println(std::format_string<Args...>&& fmt, Args&&... args)
    {
      print(std::forward<std::format_string<Args...>>(fmt), std::forward<Args>(args)...);
      println();
    }

    void println()
    {
      Rcpp::Rcout << "\n";
    }

    template<typename... Args>
    void stop(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
      Rcpp::Rcout << "ERROR: " << msg << std::endl;
      Rcpp::stop(msg);
    }

    template<typename... Args>
    void warning(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::string msg = std::vformat(fmt.get(), std::make_format_args(args...));
      Rcpp::Rcout << "Warning: " << msg << "\n";
      Rcpp::warning(msg);
    }

    auto rbinom(int const n, double const p)
      -> int
    {
      int const rv = R::rbinom(n, p);
      return rv;
    }

    // Works with array or vector input rates (maybe also Rcpp::NumericVector ??):
    template <Container C>
    [[nodiscard]] auto rmultinom(std::function<int(int const, double const)> const rbinom, int const n, C const& prob) noexcept(!Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<int>, std::array<int, C{}.size()>>
    {
      return Bridge::rmultinom(&rbinom, n, prob);
    }
 
  };

} //blofeld

#endif // BLOFELD_BRIDGE_RCPP_H
