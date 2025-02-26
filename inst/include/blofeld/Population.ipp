#ifndef BLOFELD_POPULATION_IPP
#define BLOFELD_POPULATION_IPP

#include <Rcpp.h>

namespace blofeld
{
  template<Options s_options>
  inline Population<s_options>::Population(Info const& info) noexcept
    : m_info(info)
  {
  }
  
  template<Options s_options>
  inline void Population<s_options>::update() const noexcept
  {
    int abi = m_info.get_ABI_version();
    Rcpp::Rcout << "ABI version: " << abi << "\n";
    
    Info info2(m_info);
    Info info3;
    info3 = m_info;
    Info info4 = std::move(info3);
    Info info5;
    info5 = std::move(info2);
    
  }
    
} //blofeld

#endif //BLOFELD_POPULATION_IPP
