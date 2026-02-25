#ifndef MATRIX_POPULATION_WRAPPER_H_
#define MATRIX_POPULATION_WRAPPER_H_

// We MUST have Rcpp access here, as this is a wrapper to Rcpp
#include <Rcpp.h>

#include <type_traits>

#include "../populations/matrix_population.h"

namespace blofeld
{
  
  template <auto s_cts, class MPop>
  class MatrixPopulationWrapper
  {
  private:
    using Group = MPop::GroupType;
    
    using Bridge = decltype(s_cts)::Bridge;
    Bridge m_bridge;

    // For now this has ownership - modify in future to e.g. shared pointer?
    // std::unique_ptr<MPop> m_pop;
    MPop m_pop;

  public:
    explicit MatrixPopulationWrapper(Rcpp::List groups)
    {
      // m_pop = std::make_unique<MPop>(); //(m_bridge);
      
      /*
      std::vector<Group*> vgps;
      for (index i=0; i<ssize(groups); ++i)
      {
        Group* gp = Rcpp::as<Group*>(groups[i]);
        Rcpp::Rcout << gp->val << "\n";
        //m_pop->addGroup(gp);
        vgps.push_back(gp);
      }
      m_pop->initialise(vgps);
      */
      
      for (index i=0; i<ssize(groups); ++i)
      {
        Group gp = Rcpp::as<Group>(groups[i]);
        m_pop.addGroup(gp);
      }
      
    }
    
    Group* getGroup(int num)
    {
      std::vector<Group>* vgps = m_pop.getGroups();
      if (num < 0 || num >= (*vgps).size()) Rcpp::stop("Invalid index");
      
      // Note: returning this means it can be gc() from R, and then death ensues
      Group* gp = &((*vgps)[num]);
      return gp;
    }
    
    Rcpp::List copyGroups()
    {
      std::vector<Group>* vgps = m_pop.getGroups();
      Rcpp::List rv;
      for (index i=0; i<ssize(*vgps); ++i)
      {
        rv.push_back(Rcpp::wrap((*vgps)[i]));
      }
      return rv;
    }
    
    void test()
    {
      m_pop.test();
    }
    
    void show()
    {
      m_pop.show();
    }

  };

} // blofeld

#endif // MATRIX_POPULATION_WRAPPER_H_
