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

    // Note: identical to bridge_cpp definition - TODO: DRY
    template <Container C>
    [[nodiscard]] auto rmultinom(int const n, C const& prob) noexcept(!Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<int>, std::array<int, C{}.size()>>
    {
      static_assert(std::same_as<typename C::value_type, double>, "Type mis-match: container of double expected for C");
      // Get return type, which will be C<int>:
      using R = decltype(this->rmultinom(n, prob));
      
      // TODO: check sum(prob)==1 and/or re-weight for consistency with R?

      // TODO: avoid code re-use - can constexpr be conditional?
      
      if constexpr (Fixedsize<C>) {
        constexpr std::size_t s_size = prob.size();
        if constexpr (s_size == 0U) {
          R rv {};
          return rv;
        } else if constexpr (s_size == 1U) {
          R rv { n };
          return rv;
        }

        R rv{};
        int sum = 0;
        double pp = 1.0;
        for (index i = 0; i < (ssize(prob)-1); ++i)
        {
          int const tt = rbinom(n-sum, prob[i] / pp);
          rv[i+1] = tt;
          pp -= prob[i];
          sum += tt;
        }
        rv.back() = n-sum;
        return rv;
        
      } else {
        
        std::size_t const s_size = prob.size();
        if (s_size == 0U) {
          R rv { };
          return rv;
        } else if (s_size == 1U) {
          R rv { n };
          return rv;
        }

        R rv;
        rv.resize(s_size);
        int sum = 0;
        double pp = 1.0;
        for (index i = 0; i < (ssize(prob)-1); ++i)
        {
          int const tt = rbinom(n-sum, prob[i] / pp);
          rv[i+1] = tt;
          pp -= prob[i];
          sum += tt;
        }
        rv.back() = n-sum;
        return rv;
      }
    }
 
  };

} //blofeld

#endif // BLOFELD_BRIDGE_RCPP_H
