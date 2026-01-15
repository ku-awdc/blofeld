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

      using namespace Rcpp;
      
      // List should be named:
      StringVector names = nwpars.names();
      for(int i=0; i<nwpars.size(); ++i)
      {
        // TODO: nicer error for NULL names and/or any length !=1
        String nm = names[i];
        if (nm == "beta_subclin") {
          crpars.beta_subclin = nwpars[i];
        } else if (nm == "beta_clinical") {
          crpars.beta_clinical = nwpars[i];
        } else if (nm == "contact_power") {
          crpars.contact_power = nwpars[i];
        } else if (nm == "incubation") {
          crpars.incubation = nwpars[i];
        } else if (nm == "progression") {
          crpars.progression = nwpars[i];
        } else if (nm == "recovery") {
          crpars.recovery = nwpars[i];
        } else if (nm == "healing") {
          crpars.healing = nwpars[i];
        } else if (nm == "reversion") {
          crpars.reversion = nwpars[i];
        } else if (nm == "waning") {
          crpars.waning = nwpars[i];
        } else if (nm == "vaccination") {
          crpars.vaccination = nwpars[i];
        } else if (nm == "mortality_E") {
          crpars.mortality_E = nwpars[i];
        } else if (nm == "mortality_L") {
          crpars.mortality_L = nwpars[i];
        } else if (nm == "mortality_I") {
          crpars.mortality_I = nwpars[i];
        } else if (nm == "mortality_D") {
          crpars.mortality_D = nwpars[i];
        } else if (nm == "death") {
          crpars.death = nwpars[i];
        } else if (nm == "d_time") {
          crpars.d_time = nwpars[i];
        } else {
          m_bridge.stop("Unrecognised parameter name '{}'", nm.get_cstring());
        }
      }

      m_group -> set_parameters(crpars);
    }

    [[nodiscard]] auto get_state() const
      -> DataFrame
    {
      Tstate state = m_group -> get_state();

      using namespace Rcpp;

      auto tt = state.S.get_sum();
      using trcpp = std::conditional_t<
        std::is_same<typeof(tt), double>::value,
        NumericVector,
        std::conditional_t<
          std::is_same<typeof(tt), int>::value,
          IntegerVector,
          void
        >
      >;

      auto lfun = [&](auto const& cc){
        auto ptr = cc.ptr();
        trcpp rc(1);
        for(int i=0; i<ptr.size(); ++i) rc[0] += ptr[i];
        return rc;
      };

      NumericVector tm(1);
      tm[0] = state.time;

      // List with names equating to compartments and variable lengths
      DataFrame rv = DataFrame::create(
        _["Time"] = tm,
        _["S"] = lfun(state.S),
        _["E"] = lfun(state.E),
        _["L"] = lfun(state.L),
        _["I"] = lfun(state.I),
        _["D"] = lfun(state.D),
        _["R"] = lfun(state.R),
        _["V"] = lfun(state.V),
        _["M"] = lfun(state.M)
      );

      return rv;
    }

    auto get_full_state() const
      -> List
    {
      Tstate state = m_group -> get_state();

      using namespace Rcpp;

      auto tt = state.S.get_sum();
      using trcpp = std::conditional_t<
        std::is_same<typeof(tt), double>::value,
        NumericVector,
        std::conditional_t<
          std::is_same<typeof(tt), int>::value,
          IntegerVector,
          void
        >
      >;

      auto lfun = [&](auto const& cc){
        auto ptr = cc.ptr();
        trcpp rc(ptr.size());
        for(int i=0; i<ptr.size(); ++i) rc[i] = ptr[i];
        return rc;
      };

      NumericVector tm(1);
      tm[0] = state.time;

      // List with names equating to compartments and variable lengths
      List rv = List::create(
        _["Time"] = tm,
        _["S"] = lfun(state.S),
        _["E"] = lfun(state.E),
        _["L"] = lfun(state.L),
        _["I"] = lfun(state.I),
        _["D"] = lfun(state.D),
        _["R"] = lfun(state.R),
        _["V"] = lfun(state.V),
        _["M"] = lfun(state.M)
      );

      return rv;
    }

    void set_state(List state, bool const distribute)
    {
      using namespace Rcpp;
      
      // List should be named:
      StringVector names = state.names();
      for(int i=0; i<state.size(); ++i)
      {
        // TODO: nicer error for NULL names and/or any length !=1
        String nm = names[i];
        if (nm == "S") {
          m_group -> set_state(SEIDRVMZcomp::S, state[i], distribute);
        } else if (nm == "E") {
          m_group -> set_state(SEIDRVMZcomp::E, state[i], distribute);
        } else if (nm == "L") {
          m_group -> set_state(SEIDRVMZcomp::L, state[i], distribute);
        } else if (nm == "I") {
          m_group -> set_state(SEIDRVMZcomp::I, state[i], distribute);
        } else if (nm == "D") {
          m_group -> set_state(SEIDRVMZcomp::D, state[i], distribute);
        } else if (nm == "R") {
          m_group -> set_state(SEIDRVMZcomp::R, state[i], distribute);
        } else if (nm == "V") {
          m_group -> set_state(SEIDRVMZcomp::V, state[i], distribute);
        } else if (nm == "M") {
          m_group -> set_state(SEIDRVMZcomp::M, state[i], distribute);
        } else {
          m_bridge.stop("Unrecognised compartment name '{}'", nm.get_cstring());
        }
      }

    }

  };

} // blofeld

#endif // BLOFELD_GROUP_WRAPPER_H
