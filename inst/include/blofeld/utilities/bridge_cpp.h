#ifndef BLOFELD_BRIDGE_CPP_H
#define BLOFELD_BRIDGE_CPP_H

#include <random>
#include <format>
#include <iostream>

#include "./bridge.h"

namespace blofeld
{

  template<typename T_rng>
  class BridgeCpp : Bridge
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
    void print(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::cout << std::vformat(fmt.get(), std::make_format_args(args...));
    }

    template<typename... Args>
    void println(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::cout << std::vformat(fmt.get(), std::make_format_args(args...)) << "\n";
    }

    void println()
    {
      std::cout << "\n";
    }

    template<typename... Args>
    void stop(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::cout << "ERROR: " << std::vformat(fmt.get(), std::make_format_args(args...)) << std::endl;
      throw(1);
    }

    template<typename... Args>
    void warning(std::format_string<Args...> const fmt, Args&&... args)
    {
      std::cout << "Warning: " << std::vformat(fmt.get(), std::make_format_args(args...)) << "\n";
    }

    /*
    template<typename... Args>
    void print(Args... args)
    {
      std::ostringstream ss;
      using namespace std::literals;
      stream(ss, " "sv, args...);
      std::cout << ss.str();
    };

    template<typename... Args>
    void print(std::initializer_list<std::format_string<Args...>>)
    {
      std::cout << "ilist";
    };

    template<typename... Args>
    void println(Args... args)
    {
      print(args...);
      std::cout << std::endl;
    };
    */

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

  using BridgeMT19937 = BridgeCpp<std::mt19937>;

} //blofeld

#endif // BLOFELD_BRIDGE_CPP_H
