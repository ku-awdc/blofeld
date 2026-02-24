#ifndef BLOFELD_POPULATION_H
#define BLOFELD_POPULATION_H

/* TODO
- Remove default value for s_options so the user is forced to think?  Or allow a global default option set somehow?
*/

#include "Options.h"

namespace blofeld
{
  template<Options s_options>
  class Population
  {
  private:
    Info const& m_info;
    
  public:
    Population(Info const& info) noexcept;
    
    void update() const noexcept;
  };
    
} //blofeld

#include "Population.ipp"

#endif //BLOFELD_POPULATION_H
