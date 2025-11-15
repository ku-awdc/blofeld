#ifndef BLOFELD_BRIDGE_H
#define BLOFELD_BRIDGE_H

namespace blofeld
{

  // Note: this is NOT a pure virtual class because:
  // 1. templated members cannot be virtual
  // 2. we are not using runtime polymorphism anyway
  
  class Bridge
  {
  public:
    void stop(std::string_view const msg);
    void print(std::string_view const msg);
    
    template<size_t s_size>
    std::array<int, s_size+1> rmultinom(int total, std::array<double, s_size> const& probs);
  };

} //blofeld

#endif // BLOFELD_BRIDGE_H
