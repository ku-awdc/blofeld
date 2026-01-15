#ifndef BLOFELD_GROUP_WRAPPER_H
#define BLOFELD_GROUP_WRAPPER_H

// We MUST have Rcpp access here, as this is a wrapper to Rcpp
#include <Rcpp.h>

namespace blofeld
{

  template <auto s_cts, class Tgroup>
  class GroupWrapper
  {
  private:
    using Bridge = decltype(s_cts)::Bridge;
    Bridge m_bridge;

    using Tpars = Tgroup::Tpars;
    using Tstate = Tgroup::Tstate;

    using List = Rcpp::List;
    using DataFrame = Rcpp::DataFrame;

    // For now this has ownership - modify in future to e.g. shared pointer?
    std::unique_ptr<Tgroup> m_group;

  public:
    GroupWrapper()
    {
      m_group = std::make_unique<Tgroup>(m_bridge);
    }

    /*
    template <typename Tbridge>
    explicit GroupWrapper(Bridge& bridge, Tbridge& gbridge) :
      m_bridge(bridge)
    {
      // This doesn't have to be the same Bridge as used for the wrapper:
      m_group = std::make_unique<Tgroup>(gbridge);
    }
    */

    auto update(int const n_steps) ->
      DataFrame
    {
      m_group -> update(n_steps);

      DataFrame rv = get_state();
      return rv;
    }

    [[nodiscard]] auto get_parameters() const
      -> List
    {
      // Get a struct from the underlying group and convert to Rcpp::List
      SEIDRVMZpars pars = m_group -> get_parameters();
      
      using namespace Rcpp;
      List rv = List::create(
        _["beta_subclin"] = pars.beta_subclin,
        _["beta_clinical"] = pars.beta_clinical,
        _["contact_power"] = pars.contact_power,
        _["incubation"] = pars.incubation,
        _["progression"] = pars.progression,
        _["recovery"] = pars.recovery,
        _["healing"] = pars.healing,
        _["reversion"] = pars.reversion,
        _["waning"] = pars.waning,
        _["vaccination"] = pars.vaccination,
        _["mortality_E"] = pars.mortality_E,
        _["mortality_L"] = pars.mortality_L,
        _["mortality_I"] = pars.mortality_I,
        _["mortality_D"] = pars.mortality_D,
        _["death"] = pars.death,
        _["d_time"] = pars.d_time
      );
      
      return rv;
    }

    void set_parameters(List nwpars)
    {
      // Get a struct from the underlying group and over-write any values in List names, then pass back
      SEIDRVMZpars crpars = m_group -> get_parameters();
      
      // TODO
      
      m_group -> set_parameters(crpars);
    }

    [[nodiscard]] auto get_state() const
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
      //m_bridge.println("{}, {}", state.S, state.E);
      
      using namespace Rcpp;
      
      // TODO: IntegerVector if appropriate
      auto lfun = [&](auto const& cc){
        auto ptr = cc.ptr();
        NumericVector rc(cc.size());
        for(int i=0; i<cc.size(); ++i) rc[i] = ptr[i];
        return rc;
      };
      
      // List with names equating to compartments and variable lengths
      List rv = List::create(
        _["S"] = lfun(state.S),
        _["E"] = lfun(state.E)
      );
      
      return rv;
    }

    void set_state(List)
    {
      // List should be named - for each call the appropriate underlying method depending on length of element (1 or >1)
    }

  };

} // blofeld

#endif // BLOFELD_GROUP_WRAPPER_H
