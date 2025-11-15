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
    explicit BridgeCpp(T_rng rng = [](){ std::random_device r; std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()}; T_rng rng(seed); return rng; }())
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
  
    template<size_t s_size>
    auto rmultinom(int total, std::array<double, s_size> const& probs)
      -> std::array<int, s_size+1>
    {
      std::array<int, s_size+1> rv{};
      rv[0] = total;
      return rv;
    };
  
    template<>
    auto rmultinom<1>(int total, std::array<double, 1> const& probs)
      -> std::array<int, 2>
    {
      std::array<int, 2> rv{ total, 0 };
      return rv;
    };
  
  
  };
  
  using BridgeMT19937 = BridgeCpp<std::mt19937>;

} //blofeld

#endif // BLOFELD_BRIDGE_CPP_H
