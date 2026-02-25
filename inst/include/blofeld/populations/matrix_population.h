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
  private:
    std::vector<Group> m_groups {};

  public:
    
    using GroupType = Group;
    
    MatrixPopulation()
    {
      
    }
    
    // For testing:
    explicit MatrixPopulation(Group& group)
    {
      m_groups.push_back(group);
      m_groups[0].val +=1.0;
    }
    
    explicit MatrixPopulation(std::vector<Group>& groups)
    {
      m_groups = groups;
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
    
    void test()
    {
      m_groups[0].val +=5.5;
    }
    
    void show()
    {
      Rcpp::Rcout << m_groups[0].val << "\n";
    }
    
  };

} // namespace blofeld

#endif // MATRIX_POPULATION_H_
