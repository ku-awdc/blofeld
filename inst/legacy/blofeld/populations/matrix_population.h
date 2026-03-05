#ifndef MATRIX_POPULATION_H_
#define MATRIX_POPULATION_H_

#include <vector>

// For now I am using Rcpp::NumericMatrix
// #include <Rcpp>

#include "../utilities/tools.h"

/* This class takes groups and updates them using a beta matrix */

namespace blofeld
{

  template<auto s_cts, class Group>
  class MatrixPopulation
  {
  public:
    using Bridge = decltype(s_cts)::Bridge;
    
  private:
    Bridge& m_bridge;
    
    std::vector<Group> m_groups;
    std::vector<double> m_infective;
    std::vector<double> m_beta;
    
    double m_time = 0.0;
    
    MatrixPopulation() = delete;

  public:
    
    using GroupType = Group;
    
    // For testing:
    explicit MatrixPopulation(Bridge& bridge)
      : m_bridge(bridge)
    {
      setInfective();
    }

    // For testing:
    explicit MatrixPopulation(Bridge& bridge, Group& group)
      : m_bridge(bridge)
    {
      m_groups.push_back(group);
      setInfective();
    }
    
    // Transfer ownership:
    explicit MatrixPopulation(Bridge& bridge, std::vector<Group*> const& groups)
      : m_bridge(bridge)
    {
      for (index i=0; i<ssize(groups); ++i) {
        m_groups.emplace_back(*(groups[i]));
      }
      setInfective();
    }
    
    // Set beta matrix:
    void setBetaMatrix(std::vector<double> const& beta)
    {
      if (beta.size() != std::pow(m_groups.size(), 2)) {
        m_bridge.println("Incorrect matrix dimensions");
      }
      // Take a copy:
      m_beta = beta;
    }
    
    // Return pointer to a specific group:
    Group* getGroup(int num)
    {
      if (num < 0 || num >= ssize(m_groups)) {
        m_bridge.stop("Index {} out of range", num);
      }
      
      return &(m_groups[num]);
    }
    
    void setInfective()
    {
      std::vector<double> vec(std::pow(m_groups.size(), 2), 0.0);
      setBetaMatrix(vec);
      
      m_infective.resize(m_groups.size());
      updateInfective();      
    }
    
    void updateInfective()
    {
      for (index i=0; i<ssize(m_groups); ++i) {
        m_infective[i] = static_cast<double>(m_groups[i].getInfective());
      }
    }
    
    void update_one(int substeps = 1)
    {
      // First refresh the number of infective:
      updateInfective();
      m_time += static_cast<double>(substeps);
      
      // And then deal with each group:
      index const dd = ssize(m_groups);
      for (index i=0; i<dd; ++i) {
        // Calculate extbeta:
        double extb = 0.0;
        for (index j=0; j<dd; ++j) {
          extb += m_infective[j] * m_beta[j*dd+i];
        }        
        // Set extbeta and update:
        m_groups[i].set_external_infection(extb);
        m_groups[i].update(substeps);
      }
    }
    
    void update(int const steps, int const substeps = 1)
    {
      for (int i=0; i<steps; ++i) {
        update_one(substeps);
        Rcpp::checkUserInterrupt();
      }
    }
    
    auto getState() const
    {
      // Get total numbers:
      struct
      {
        double Time = 0.0;
        double S = 0.0;
        double E = 0.0;
        double L = 0.0;
        double I = 0.0;
        double D = 0.0;
        double R = 0.0;
        double V = 0.0;
        double M = 0.0;
      } rv;
      
      
      bool time_ok = true;
      for (index i=0; i<ssize(m_groups); ++i) {
        auto const state = m_groups[i].get_state();
        if (i > 0 && !identical(state.time, rv.Time)) {
          time_ok = false;
        }
        rv.Time = state.time;
        rv.S += state.S.get_sum();
        rv.E += state.E.get_sum();
        rv.L += state.L.get_sum();
        rv.I += state.I.get_sum();
        rv.D += state.D.get_sum();
        rv.R += state.R.get_sum();
        rv.V += state.V.get_sum();
        rv.M += state.M.get_sum();
      }
      
      if (!time_ok) {
        m_bridge.warning("Inconsistent time value in groups");
      }
      
      return rv;
    }
    
    void show()
    {
      m_bridge.println("MatrixPopulation of {} groups", m_groups.size());
    }
    
  };

} // namespace blofeld

#endif // MATRIX_POPULATION_H_
