#ifndef BLOFELD_BRIDGE_RCPP_H
#define BLOFELD_BRIDGE_RCPP_H

#include <format>
#include <iostream>

#include <Rcpp.h>
#define R_NO_REMAP

#include "./bridge.h"

namespace blofeld
{

  class BridgeRcpp : Bridge
  {
  private:

  public:
    explicit BridgeRcpp()
    {

    }

    template<typename... Args>
    void print(std::format_string<Args...> const fmt, Args&&... args)
    {
      Rcpp::Rcout << std::vformat(fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args>
    void println(std::format_string<Args...> const fmt, Args&&... args)
    {
      Rcpp::Rcout << std::vformat(fmt.get(), std::make_format_args(args...)) << "\n";
    }

    void println()
    {
      Rcpp::Rcout << "\n";
    }

    template<typename... Args>
    void stop(std::format_string<Args...> const fmt, Args&&... args)
    {
      Rcpp::Rcout << "ERROR: " << std::vformat(fmt.get(), std::make_format_args(args...)) << std::endl;
      Rcpp::stop(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    template<typename... Args>
    void warning(std::format_string<Args...> const fmt, Args&&... args)
    {
      Rcpp::Rcout << "Warning: " << std::vformat(fmt.get(), std::make_format_args(args...)) << "\n";
      Rcpp::warning(std::vformat(fmt.get(), std::make_format_args(args...)));
    }

    auto rbinom(int const n, double const p)
      -> int
    {
      int const rv = R::rbinom(n, p);
      return rv;
    }

    template<size_t s_size>
    auto rmultinom(int n, std::array<double, s_size> const& prob)
      -> std::array<int, s_size+1>
    {
      if constexpr (s_size == 0U)
      {
        std::array<int, 1> rv{ n };
        return rv;
      }
      
      // TODO: check sum(prob)<=1

      std::array<int, s_size+1> rv{};
      int sum = 0;
      double pp = 1.0;
      for (auto i = 0; i < s_size; ++i)
      {
        int const tt = rbinom(n-sum, prob[i] / pp);
        rv[i+1] = tt;
        pp -= prob[i];
        sum += tt;
      }
      rv[0] = n-sum;
      return rv;
    }

  };

} //blofeld

#endif // BLOFELD_BRIDGE_RCPP_H
