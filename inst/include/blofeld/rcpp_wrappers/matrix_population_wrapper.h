#ifndef MATRIX_POPULATION_WRAPPER_H_
#define MATRIX_POPULATION_WRAPPER_H_

// We MUST have Rcpp access here, as this is a wrapper to Rcpp
#include <Rcpp.h>

#include <type_traits>

#include "../populations/matrix_population.h"

namespace blofeld
{

  template <class MPop>
  class MatrixPopulationWrapper
  {
  private:
    using Bridge = MPop::Bridge;
    using Group = MPop::GroupType;
    using GpWp = GroupWrapper<Group>;

    Bridge m_bridge;

    // For now this has ownership - modify in future to e.g. shared pointer?
    std::unique_ptr<MPop> m_pop;

  public:
    explicit MatrixPopulationWrapper(Rcpp::List wrapped_groups)
    {

      // Transfer ownership from inside the group wrappers:
      std::vector<Group*> vgps;
      for (index i=0; i<ssize(wrapped_groups); ++i)
      {
        GpWp* gw = Rcpp::as<GpWp*>(wrapped_groups[i]);;
        Group* gp = gw->getPtr();
        vgps.push_back(gp);
      }

      m_pop = std::make_unique<MPop>(m_bridge, vgps);

      int const num = 0;
      Group* gp = m_pop->getGroup(num);

      gp->update(10);

      GpWp* gw = Rcpp::as<GpWp*>(wrapped_groups[0]);;
      gw->changePtr(gp);

      //Group* gp = groups[0].getPtr();

      // Transfer ownership from inside the group wrappers:
      /*
      std::vector<Group*> vgps;
      for (index i=0; i<ssize(groups); ++i)
      {
        Group* gp = Rcpp::as<Group*>(groups[i]);
        vgps.push_back(gp);
      }

      m_pop = std::make_unique<MPop>(m_bridge, vgps);
      */

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

      for (index i=0; i<ssize(groups); ++i)
      {
        Group gp = Rcpp::as<Group>(groups[i]);
        m_pop.addGroup(gp);
      }

      */
    }

    GpWp getGroup(int num)
    {
      Group* gp = m_pop->getGroup(num);
      GpWp gw(gp);
      
      return gw;
    }

    /*


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
      m_pop->test();
    }

    */

    void show()
    {
      //m_pop->show();
    }

  };

} // blofeld

#endif // MATRIX_POPULATION_WRAPPER_H_
