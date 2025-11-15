#ifndef BLOFELD_BRIDGE_CPP_H
#define BLOFELD_BRIDGE_CPP_H

#include <random>

#include "Bridge.h"

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
  
    void stop(std::string_view const msg)
    {
      throw(msg);
    };
  
    void print(std::string_view const msg)
    {
      std::cout << msg;
    };
    
    void println(std::string_view const msg)
    {
      std::cout << msg << "\n";
    };

    template<typename T>
    void println(T const msg)
    {
      std::cout << msg << "\n";
    };
    
    auto rbinom(int const n, double const p)
      -> int
    {
      // TODO: check n and p
      
      if(n==0) return(0);
      std::binomial_distribution<> d(n, p);
      return d(m_rng);
    }
  
    template<size_t s_size>
    auto rmultinom(int n, std::array<double, s_size> const& prob)
      -> std::array<int, s_size+1>
    {
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
    };
  
    template<>
    auto rmultinom<0>(int total, std::array<double, 0> const& probs)
      -> std::array<int, 1>
    {
      std::array<int, 1> rv{ total };
      return rv;
    };
  
  
  };
  
  using BridgeMT19937 = BridgeCpp<std::mt19937>;

} //blofeld

#endif // BLOFELD_BRIDGE_CPP_H
