#ifndef BLOFELD_GROUP_WRAPPER_H
#define BLOFELD_GROUP_WRAPPER_H

// We MUST have Rcpp access here, as this is a wrapper to Rcpp
// #include <Rcpp.h>

namespace blofeld
{

  template <auto s_cts, class Tgroup>
  class GroupWrapper
  {
  private:
    using Bridge = decltype(s_cts)::Bridge;
    Bridge& m_bridge;
    
    using Tpars = Tgroup::Tpars;
    using Tstate = Tgroup::Tstate;
    
    using List = double;
    using DataFrame = double;

    // For now this has ownership - modify in future to e.g. shared pointer?
    std::unique_ptr<Tgroup> m_group;
    
  public:
    template <typename Tbridge>
    explicit GroupWrapper(Bridge& bridge, Tbridge& gbridge) :
      m_bridge(bridge)
    {
      // This doesn't have to be the same Bridge as used for the wrapper:
      m_group = std::make_unique<Tgroup>(gbridge);
    }
    
    void update(int const n_steps)
    {
      m_group -> update(n_steps);
      m_bridge.println("DONE");
    }
    
    auto get_parameters() const
      -> List
    {
      // Get a struct from the underlying group and convert to Rcpp::List
    }
    
    void set_parameters(List)
    {
      // Get a struct from the underlying group and over-write any values in List names, then pass back
    }
    
    auto get_state() const
      -> DataFrame 
    {
      get_full_state();
      // Data frame with single row and columns for each compartment (total)
      DataFrame rv = 1.0;
      return rv;
    }

    auto get_full_state() const
      -> List
    {
      Tstate state = m_group -> get_state();
      m_bridge.println("{}, {}", state.S, state.E);
      
      // List with names equating to compartments and variable lengths
      List rv = 1.0;
      return rv;
    }

    void set_state(List)
    {
      // List should be named - for each call the appropriate underlying method depending on length of element (1 or >1)
    }
    
  };

} // blofeld

#endif // BLOFELD_GROUP_WRAPPER_H
