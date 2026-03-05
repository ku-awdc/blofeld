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

      for (index i=0; i<ssize(wrapped_groups); ++i)
      {
        Group* gp = m_pop->getGroup(i);
        GpWp* gw = Rcpp::as<GpWp*>(wrapped_groups[i]);;
        gw->changePtr(gp);
      }

      m_bridge.println("WARNING: Ownership of {} groups has been transferred to the matrix population and the R objects representing these groups now point inside the matrix population. Do NOT use these group objects after the metapopulation has been gc'd otherwise very bad things will happen.", ssize(wrapped_groups));

    }

    GpWp getGroup(int num)
    {
      Group* gp = m_pop->getGroup(num);
      GpWp gw(gp);

      return gw;
    }

    Rcpp::DataFrame update(int const steps)
    {
      m_pop->update(steps);
      return getState();
    }
    
    Rcpp::DataFrame getState() const
    {
      auto const state = m_pop->getState();
      
      using namespace Rcpp;
      
      DataFrame rv = DataFrame::create(
        _["TimePoint"] = state.Time,
        _["S"] = state.S,
        _["E"] = state.E,
        _["L"] = state.L,
        _["I"] = state.I,
        _["D"] = state.D,
        _["R"] = state.R,
        _["V"] = state.V,
        _["M"] = state.M
      );

      return rv;      
    }

    void setBetaMatrix(Rcpp::NumericMatrix beta)
    {
      if (beta.nrow() != beta.ncol()) {
        m_bridge.stop("Matrix is not symmetric");
      }
      index const dd = symmetric_cast<index>(beta.nrow());

      std::vector<double> vec;
      for (index i = 0; i < dd; ++i) {
        for (index j = 0; j < dd; ++j) {
          double const val = beta(i,j);
          if ( val < 0.0 ) {
            m_bridge.stop("Invalid negative entry in beta matrix");
          }
          vec.push_back(val);
        }
      }
      m_pop->setBetaMatrix(vec);
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
      m_pop->show();
    }

  };

} // blofeld

#endif // MATRIX_POPULATION_WRAPPER_H_
