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
    
    std::vector<Group> m_groups {};
    
    MatrixPopulation() = delete;

  public:
    
    using GroupType = Group;
    
    // For testing:
    explicit MatrixPopulation(Bridge& bridge)
      : m_bridge(bridge)
    {
    }

    // For testing:
    explicit MatrixPopulation(Bridge& bridge, Group& group)
      : m_bridge(bridge)
    {
      m_groups.push_back(group);
    }
    
    // Transfer ownership:
    explicit MatrixPopulation(Bridge& bridge, std::vector<Group*> const& groups)
      : m_bridge(bridge)
    {
      m_bridge.println("Transferring {} groups", groups.size());
      for (index i=0; i<ssize(groups); ++i) {
        Group gp = *(groups[i]);
        m_groups.push_back(gp);
      }
    }
    
    // Return pointer to a specific group:
    Group* getGroup(index num)
    {
      // TODO:
      if (num < 0 || num >= ssize(m_groups)) {
        m_bridge.stop("Index {} out of range", num);
      }
      
      return &(m_groups[num]);
    }
    
    /*
    explicit MatrixPopulation(Rcpp::List groups)
    {
      m_groups.resize(groups.size());      
      for (index i=0; i<ssize(groups); ++i)
      {
        Group* gp = Rcpp::as<Group*>(groups[0]);
        m_groups[i] = gp;
      }
      m_groups[0]->val +=20.0;
    }*/
    
    void initialise(std::vector<Group> groups)
    {
      m_groups = groups;
    }
    
    void addGroup(Group& gp)
    {
      m_groups.push_back(gp);
    }
    
    std::vector<Group>* getGroups()
    {
      return &m_groups;
    }
    
    void show()
    {
      m_bridge.println("MatrixPopulation of {}", m_groups.size());
    }
    
  };

} // namespace blofeld

#endif // MATRIX_POPULATION_H_
