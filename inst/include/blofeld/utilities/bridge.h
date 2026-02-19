#ifndef BLOFELD_BRIDGE_H
#define BLOFELD_BRIDGE_H

#include <string_view>
#include <functional>
#include <type_traits>

namespace blofeld
{

  template <typename T>
  concept Ostream = std::is_base_of<std::ostream, T>::value;
  
  class Bridge
  {
  protected:
    
    // This is just a base class:
    Bridge()
    {
      
    }
    
    template<Ostream O, typename... Args>
    void print(O& out, std::format_string<Args...> const fmt, Args&&... args)
    {
      out << std::vformat(fmt.get(), std::make_format_args(args...));
    }
    
    
    // Works with array or vector input rates (maybe also Rcpp::NumericVector ??):
    template <Container C, typename F>
//    [[nodiscard]] auto rmultinom(std::function<int(int const, double const)> rbinom, int const n, C const& prob) noexcept(!Resizeable<C>)
    [[nodiscard]] auto rmultinom(F rbinom, int const n, C const& prob) noexcept(!Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<int>, std::array<int, C{}.size()>>
    {
      static_assert(std::same_as<typename C::value_type, double>, "Type mis-match: container of double expected for C");
      // Get return type, which will be C<int>:
      using R = decltype(this->rmultinom(rbinom, n, prob));
      
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

#endif // BLOFELD_BRIDGE_H

