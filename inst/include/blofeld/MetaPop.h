#ifndef BLOFELD_META_POP_H_
#define BLOFELD_META_POP_H_

#include <tuple>
#include <iostream>

template <class... T_pops>
class MetaPop
{
private:
  typedef std::tuple<T_pops...> T_mptype;
  T_mptype m_pops;
  static constexpr size_t m_numpop = std::tuple_size<T_mptype>{};
  
  template<size_t t_i = 0>
  void print()
  {
    constexpr size_t popnum = t_i+1;
    std::cout << popnum << ": " << std::get<t_i>(m_pops) << ", ";
    
    // Note: checking next index, which is the same as current number:
    if constexpr (popnum < m_numpop)
        print<popnum>();
  }
  
  MetaPop() = delete;
  
public:
    
  MetaPop(T_pops... pops)
    : m_pops(pops...)
  {
    std::cout << m_numpop << "\n";
  }
  
  void dostuff()
  {
    print();
    std::cout << "\n";
  }
  
};


#endif //BLOFELD_META_POP_H_