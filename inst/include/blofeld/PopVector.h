#ifndef POP_VECTOR_H_
#define POP_VECTOR_H_

#include <vector>

template<class t_Group, class t_Pars>
class PopVector
{
private:
  typedef HAIstate<t_debug> t_state;

  std::vector<t_Group> m_groups;
  std::vector<t_Group> m_active_states; // typeof t_Group::get_state
  Rcpp::DataFrame m_all_states; // states needs to have a method to write into a data frame with specified row
  std::vector<t_Pars> m_pars;

  t_state m_state;

public:
  PopVector(int N) :
    m_state(N)
  {

    t_Pars<t_hr> pars;

    if constexpr (t_hr)
    {
      Rprintf("MAKING A HR POP\n");

      pars = { 10, 0.0, 0.0, 0.0};
    }else{
      Rprintf("MAKING A LR POP\n");

      pars = { 10, 0.0, 0.0};
    }

    m_wards.reserve(N);
    for(int i=0; i<N; ++i)
    {
      m_wards.emplace_back(0, pars);
      m_wards.back().set_freq(1.0);
      if constexpr (t_hr)
      {
        m_wards.back().set_dens(1.0);
      }
    }
  }

  int number()
  {
    return m_wards.size();
  }

  void infect()
  {
    m_state.do_smth();
  }

  auto const& get_state() const
  {
    return m_state;
  }

  ~PopVector()
  {
    if constexpr (t_debug)
    {
      Rprintf("DELETING A POP\n");
    }
  }
};

#endif // POP_VECTOR_H_
