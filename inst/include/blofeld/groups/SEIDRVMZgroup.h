#ifndef BLOFELD_SEIDRVMZ_GROUP_H
#define BLOFELD_SEIDRVMZ_GROUP_H

#include "Group.h"
#include "ModelCompTypes.h"
#include "Compartment.h"

namespace blofeld
{
  
  enum class SEIDRVMZcomp
  {
    S, E, L, I, D, R, V, M
  };
  
  struct SEIDRVMZpars
  {
    // For getting and setting all parameter values
    double beta_subclin = 0.0;    // Beta for L animals
    double beta_clinical = 0.0;   // Beta for I animals
    double contact_power = 1.0;   // Frequency vs density vs other dependence ( beta * S * I / N^contact_power)
      
    double incubation = 0.0;      // From E (exposed, not infectious)
    double progression = 0.0;     // From L (infectious, not clinical)
    double recovery = 0.0;        // From I (infectious and clinical)
    double healing = 0.0;         // From D (not infectious but clinical)
    double reversion = 0.0;       // From R (immune)
    double waning = 0.0;          // From V (vaccinated)

    double vaccination = 0.0;     // Random vaccination rate (S, R, V only) - TODO: remove
    double mortality_E = 0.0;     // Disease-related mortality for Es
    double mortality_L = 0.0;     // Disease-related mortality for Ls
    double mortality_I = 0.0;     // Disease-related mortality for Is
    double mortality_D = 0.0;     // Disease-related mortality for Ds
    double death = 0.0;           // Other-cause mortality (also for E/L/I/D)
    
    double d_time = 1.0;          // Time step
  };
  
  // TODO: have an inits struct to set Ns etc

  template <auto s_cts, ModelType s_mtype, CompType s_ctp_S, CompType s_ctp_E, CompType s_ctp_L, CompType s_ctp_I, CompType s_ctp_D, CompType s_ctp_R, CompType s_ctp_V, CompType s_ctp_M, CompType s_ctp_Z>
  struct SEIDRVMZstate
  {
    // For getting only:  always full state
    // Setting is done for specific compartments separately
    double time = 0.0;
    Compartment<s_cts, s_mtype, s_ctp_S> S;   // Susceptible
    Compartment<s_cts, s_mtype, s_ctp_E> E;   // Exposed but not infectious or clinical
    Compartment<s_cts, s_mtype, s_ctp_L> L;   // Infectious but not clinical
    Compartment<s_cts, s_mtype, s_ctp_I> I;   // Infectious and clinical
    Compartment<s_cts, s_mtype, s_ctp_D> D;   // Not infectious but clinical
    Compartment<s_cts, s_mtype, s_ctp_R> R;   // Recovered - immune from reinfection
    Compartment<s_cts, s_mtype, s_ctp_V> V;   // Vaccinated - immune with probability ve_susceptible
    Compartment<s_cts, s_mtype, s_ctp_M> M;   // Dead from disease
  };

  template <auto s_cts, ModelType s_mtype, CompType s_ctp_S, CompType s_ctp_E, CompType s_ctp_L, CompType s_ctp_I, CompType s_ctp_D, CompType s_ctp_R, CompType s_ctp_V, CompType s_ctp_M, CompType s_ctp_Z>
  class SEIDRVMZgroup : public Group<s_cts>
  {
  private:

    // TODO: make this a static consteval member of ModelType for re-use here and Compartment?
    using t_Value = std::conditional_t<
      s_mtype == ModelType::deterministic,
      double,
      std::conditional_t<
        s_mtype == ModelType::stochastic,
        int,
        void
      >
    >;

    using Bridge = decltype(s_cts)::Bridge;
    Bridge& m_bridge;
    
    double m_time = 0.0;
    Compartment<s_cts, s_mtype, s_ctp_S> m_S;
    Compartment<s_cts, s_mtype, s_ctp_E> m_E;
    Compartment<s_cts, s_mtype, s_ctp_L> m_L;
    Compartment<s_cts, s_mtype, s_ctp_I> m_I;
    Compartment<s_cts, s_mtype, s_ctp_D> m_D;
    Compartment<s_cts, s_mtype, s_ctp_R> m_R;
    Compartment<s_cts, s_mtype, s_ctp_V> m_V;
    Compartment<s_cts, s_mtype, s_ctp_M> m_M;
    Compartment<s_cts, s_mtype, s_ctp_Z> m_Z;
    
    static constexpr bool s_have_death = s_ctp_Z.is_active();
    static constexpr bool s_have_vacc = s_ctp_V.is_active();
    static constexpr bool s_have_mort = s_ctp_M.is_active();
    
    // Parameters:
    double m_beta_subclin = 0.0;
    double m_beta_clinical = 0.0;
    double m_contact_power = 1.0;      
    double m_incubation = 0.0;
    double m_progression = 0.0;
    double m_recovery = 0.0;
    double m_healing = 0.0;
    double m_reversion = 0.0;
    double m_waning = 0.0;
    double m_vaccination = 0.0;
    double m_mortality_E = 0.0;
    double m_mortality_L = 0.0;
    double m_mortality_I = 0.0;
    double m_mortality_D = 0.0;
    double m_death = 0.0;
    
    SEIDRVMZpars m_pars {
      .beta_subclin = m_beta_subclin,
      .beta_clinical = m_beta_clinical,
      .contact_power = m_contact_power,
      .incubation = m_incubation,
      .progression = m_progression,
      .recovery = m_recovery,
      .healing = m_healing,
      .reversion = m_reversion,
      .waning = m_waning,
      .vaccination = m_vaccination,
      .mortality_E = m_mortality_E,
      .mortality_L = m_mortality_L,
      .mortality_I = m_mortality_I,
      .mortality_D = m_mortality_D,
      .death = m_death,
      .d_time = 1.0
    };
    
    double m_external_infection = 0.0;
    
    // Do we have death?
    static constexpr size_t s_psd = [](){
      if constexpr (s_have_death) {
        return 1;
      } else {
        return 0;
      }
    }();
    
    // The expected array size from process_rate for non-EID compartments:
    static constexpr size_t s_psv = [](){
      if constexpr (s_have_vacc) {
        return s_psd+1;
      } else {
        return s_psd;
      }
    }();

    // The expected array size from process_rate for EID compartments:
    static constexpr size_t s_psm = [](){
      if constexpr (s_have_mort) {
        return s_psd+1;
      } else {
        return s_psd;
      }
    }();

    // Death is always first, then vaccine (but for R it restarts R, not goes to V)
    std::array<double,s_psv> m_deathvacc_S_rate {};
    std::array<double,s_psv> m_deathvacc_R_rate {};
    std::array<double,s_psv> m_deathvacc_V_rate {};
    
    // Death is always first, then mortality/cull:
    std::array<double,s_ctp_E.is_active() ? s_psm : 0> m_deathmort_E_rate {};
    std::array<double,s_ctp_L.is_active() ? s_psm : 0> m_deathmort_L_rate {};
    std::array<double,s_ctp_I.is_active() ? s_psm : 0> m_deathmort_I_rate {};
    std::array<double,s_ctp_D.is_active() ? s_psm : 0> m_deathmort_D_rate {};

  public:
    
    using Tpars = SEIDRVMZpars;
    using Tstate = SEIDRVMZstate<s_cts, s_mtype, s_ctp_S, s_ctp_E, s_ctp_L, s_ctp_I, s_ctp_D, s_ctp_R, s_ctp_V, s_ctp_M, s_ctp_Z>;
    
    SEIDRVMZgroup(Bridge& bridge)
      : m_bridge(bridge), m_S(bridge), m_E(bridge), m_L(bridge), m_I(bridge),
        m_D(bridge), m_R(bridge), m_V(bridge), m_M(bridge), m_Z(bridge)
    {
      validate();
    }
    
    void validate() const
    {
      // TODO: checks that we always have an S
      // TODO: check that Z is a balancing type with n=0 or n=1, and none of the others are balancing
      
    }
    
    void set_parameters(SEIDRVMZpars const& pars)
    {
      m_pars = pars;
      
      m_beta_subclin = pars.beta_subclin * pars.d_time;
      m_beta_clinical = pars.beta_clinical * pars.d_time;
      m_contact_power = pars.contact_power * pars.d_time;
      m_incubation = pars.incubation * pars.d_time;
      m_progression = pars.progression * pars.d_time;
      m_recovery = pars.recovery * pars.d_time;
      m_healing = pars.healing * pars.d_time;
      m_reversion = pars.reversion * pars.d_time;
      m_waning = pars.waning * pars.d_time;
      m_vaccination = pars.vaccination * pars.d_time;
      m_mortality_E = pars.mortality_E * pars.d_time;
      m_mortality_L = pars.mortality_L * pars.d_time;
      m_mortality_I = pars.mortality_I * pars.d_time;
      m_mortality_D = pars.mortality_D * pars.d_time;
      m_death = pars.death * pars.d_time;
      
      validate();      
    }

    auto get_parameters() const
      -> SEIDRVMZpars
    {
      validate();
      
      return m_pars;
    }

    void set_external_infection(double const extinf)
    {
      m_external_infection = extinf;
    }
    
    auto get_external_infection() const
      -> double
    {
      return m_external_infection;
    }
    
    auto get_state() const
      -> Tstate
    {
      Tstate state { m_time, m_S, m_E, m_L, m_I, m_D, m_R, m_V, m_M };
      return state;
    }
    
    void set_state(SEIDRVMZcomp compartment, t_Value value, bool distribute)
    {
      if (compartment == SEIDRVMZcomp::S) {
        m_S.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::E) {
        m_E.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::L) {
        m_L.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::I) {
        m_I.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::D) {
        m_D.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::R) {
        m_R.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::V) {
        m_V.set_sum(value, distribute);
      } else if (compartment == SEIDRVMZcomp::M) {
        m_M.set_sum(value, distribute);
      } else {
        m_bridge.stop("Unrecognised compartment value in set_state");
      }
      
      m_Z.set_sum(m_S.get_sum() + m_E.get_sum() + m_L.get_sum() + m_I.get_sum() + m_D.get_sum() + m_R.get_sum() + m_V.get_sum() + m_M.get_sum());
      if constexpr (s_cts.debug) { validate(); }      
    }
    
    void update(int const n_steps = 1)
    {
      if constexpr (s_cts.debug) { validate(); }
      for (int i=0; i<n_steps; ++i)
      {
        update_one();
      }
      if constexpr (s_cts.debug) { validate(); }
    }
    
    void update_one()
    {
      m_time += m_pars.d_time;
      
      // TODO: calculate only when contact power or Z/M change:
      double const freqdens = static_cast<double>(std::pow((m_Z.get_sum() - m_M.get_sum()), m_contact_power));
      // TODO: add external infection
      double const inf_rate = m_external_infection + ((m_beta_subclin * static_cast<double>(m_L.get_sum()) + m_beta_clinical * static_cast<double>(m_I.get_sum())) / freqdens);
      
      // We always have S:
      auto const S_carry = [&](){
        auto const [carry, take] = m_S.process_rate(inf_rate, m_deathvacc_S_rate);
        if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
        if constexpr (s_have_vacc) m_V.insert_value_start(take[s_have_death ? 1 : 0]);
        return carry;
      }();

      // We don't always have E:
      auto const E_carry = [&](auto const input){
        if constexpr (s_ctp_E.is_active()) {        
          m_E.insert_value_start(input);
          auto const [carry, take] = m_E.process_rate(m_incubation, m_deathmort_E_rate);
          if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
          if constexpr (s_have_mort) m_M.insert_value_start(take[s_have_death ? 1 : 0]);
          return carry;
        } else {
          return input;
        }
      }(S_carry);

      // We don't always have L:
      auto const L_carry = [&](auto const input){
        if constexpr (s_ctp_L.is_active()) {        
          m_L.insert_value_start(input);
          auto const [carry, take] = m_L.process_rate(m_progression, m_deathmort_L_rate);
          if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
          if constexpr (s_have_mort) m_M.insert_value_start(take[s_have_death ? 1 : 0]);
          return carry;
        } else {
          return input;
        }
      }(E_carry);

      // We don't always have I:
      auto const I_carry = [&](auto const input){
        if constexpr (s_ctp_I.is_active()) {        
          m_I.insert_value_start(input);
          auto const [carry, take] = m_I.process_rate(m_recovery, m_deathmort_I_rate);
          if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
          if constexpr (s_have_mort) m_M.insert_value_start(take[s_have_death ? 1 : 0]);
          return carry;        
        } else {
          return input;
        }
      }(L_carry);

      // We don't always have D:
      auto const D_carry = [&](auto const input){
        if constexpr (s_ctp_D.is_active()) {        
          m_D.insert_value_start(input);
          auto const [carry, take] = m_D.process_rate(m_healing, m_deathmort_D_rate);
          if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
          if constexpr (s_have_mort) m_M.insert_value_start(take[s_have_death ? 1 : 0]);
          return carry;        
        } else {
          return input;
        }
      }(I_carry);

      // We don't always have R:
      auto const R_carry = [&](auto const input){
        if constexpr (s_ctp_R.is_active()) {        
          m_R.insert_value_start(input);
          auto const [carry, take] = m_R.process_rate(m_reversion, m_deathvacc_R_rate);
          if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
          // Note: deliberately restart R rather than go to V for vaccine effect:
          if constexpr (s_have_vacc) m_R.insert_value_start(take[s_have_death ? 1 : 0]);
          return carry;        
        } else {
          return input;
        }
      }(D_carry);


      // TODO: allow V to become infected      
      auto const V_carry = [&](){
        if constexpr (s_ctp_V.is_active()) {        
          auto const [carry, take] = m_V.process_rate(m_waning, m_deathvacc_V_rate);
          if constexpr (s_have_death) m_Z.insert_value_start(take[0]);
          // Note: restart V if re-vaccinated:
          if constexpr (s_have_vacc) m_V.insert_value_start(take[s_have_death ? 1 : 0]);
          return carry;        
        } else {
          return static_cast<t_Value>(0.0);
        }
      }();
      
      m_S.insert_value_start(V_carry + R_carry);
        
      m_S.apply_changes();
      if constexpr (s_ctp_E.is_active()) m_E.apply_changes();
      if constexpr (s_ctp_L.is_active()) m_L.apply_changes();
      if constexpr (s_ctp_I.is_active()) m_I.apply_changes();
      if constexpr (s_ctp_D.is_active()) m_D.apply_changes();
      if constexpr (s_ctp_R.is_active()) m_R.apply_changes();
      if constexpr (s_ctp_V.is_active()) m_V.apply_changes();
      if constexpr (s_ctp_M.is_active()) m_M.apply_changes();
      m_Z.apply_changes();
      
      if constexpr (s_cts.debug) {
        auto const total = m_S.get_sum() + m_E.get_sum() + m_L.get_sum() + m_I.get_sum() + m_D.get_sum() + m_R.get_sum() + m_V.get_sum() + m_M.get_sum();
        // m_bridge.println("S = {}; I = {}; R = {}; Z = {}", m_S.get_sum(), m_I.get_sum(), m_R.get_sum(), m_Z.get_sum());
      
        if (std::abs(m_Z.get_sum() - total) > s_cts.tol) {
          m_bridge.stop("Imbalance detected: total = {}; Z = {}", total, m_Z.get_sum());
        }
      }

    }
  
};
  
}

/*
    double m_Z = 0.0;
    double m_sumTx = 0.0;
    double m_sumVx = 0.0;
    double m_sumRx = 0.0;
    double m_sumMx = 0.0;

    int m_day = 0;
    double m_time = 0.0;

    double m_vs_rate = 0.0;       // #1
    double m_ni_rate = 0.0;       // #2
    double m_rs_rate = 0.0;       // #3
    double m_beta = 0.0;          // #4
    double m_ia_rate = 0.0;       // #5
    double m_screc_prop = 0.0;    // #6
    double m_disrec_prop = 0.0;   // #7
    double m_birthrate = 0.0;     // #8
    double m_ac_rate = 0.0;       // #9
    double m_mort_nat = 0.0;      // #10
    double m_mort_acute = 0.0;    // #11
    double m_mort_chronic = 0.0;  // #11
    double m_rel_fecundity = 0.0; // #12

    double m_se = 1.0;
    double m_sp = 1.0;
    // Destinations: (1) R, (2) N, (3) A/C/I, (4) Z
    double m_cure_N = 0.9;
    double m_cure_I = 0.9;
    double m_cure_A = 0.75;
    double m_cure_C = 0.6;
    double m_vx_eff = 1.0;
    double m_vx_bst = 1.0;
    double m_passive_rate = 0.0;

    // Assume that passive cull rates are fixed and hard-coded for ease:
    static constexpr double const s_passive_cull_positive = 0.0;
    static constexpr double const s_passive_cull_acute = 0.2;
    static constexpr double const s_passive_cull_chronic = 0.3;

    bool m_recording = false;

    KoalaGroup() = delete;

    [[nodiscard]] auto to_duration(double const rate)
      const noexcept(!CTS.debug)
      -> double
    {
      if (rate == 0.0) return R_PosInf;

      double const duration = 1.0 / (rate * 365.0);
      return duration;
    }

    [[nodiscard]] auto to_rate(double const duration) const noexcept(!CTS.debug)
      -> double
    {
      if (Rcpp::traits::is_infinite<REALSXP>(duration)) return 0.0;

      double const rate = 1.0 / (duration * 365.0);
      return rate;
    }

    auto update_death(double const d_time) noexcept(!CTS.debug)
      -> void
    {
      m_Z += m_S.take_rate(m_mort_nat, d_time);
      m_Z += m_V.take_rate(m_mort_nat, d_time);
      m_Z += m_I.take_rate(m_mort_nat, d_time);
      m_Z += m_N.take_rate(m_mort_nat, d_time);
      m_Z += m_R.take_rate(m_mort_nat, d_time);
      m_Z += m_Sf.take_rate(m_mort_nat, d_time);
      m_Z += m_Vf.take_rate(m_mort_nat, d_time);
      m_Z += m_If.take_rate(m_mort_nat, d_time);
      m_Z += m_Nf.take_rate(m_mort_nat, d_time);
      m_Z += m_Rf.take_rate(m_mort_nat, d_time);

      // Mortality for Af and Cf is different:
      double const dmort = m_Af.take_rate(m_mort_acute, d_time) + m_Cf.take_rate(m_mort_chronic, d_time);
      m_sumMx += dmort;
      m_Z += dmort;

      update_apply();
    }

    auto update_birth(double const d_time) noexcept(!CTS.debug)
      -> void
    {
      double const births = m_birthrate * (get_fertile() + (m_rel_fecundity * get_infertile())) * d_time;
      // Note: this is correct to be value not rate!
      m_S.insert_value_start(births);
      m_Z -= births;

      update_apply();
    }

    auto update_disease(double const d_time) noexcept(!CTS.debug)
      -> void
    {
      // V(f) -> S(f)
      m_S.insert_value_start( m_V.carry_rate(m_vs_rate, d_time) );
      m_Sf.insert_value_start( m_Vf.carry_rate(m_vs_rate, d_time) );

      // N(f) -> I(f)
      m_I.insert_value_start( m_N.carry_rate(m_ni_rate, d_time) );
      m_If.insert_value_start( m_Nf.carry_rate(m_ni_rate, d_time) );

      // R(f) -> S(f)
      m_S.insert_value_start( m_R.carry_rate(m_rs_rate, d_time) );
      m_Sf.insert_value_start( m_Rf.carry_rate(m_rs_rate, d_time) );

      // S(f) -> I(f)
      {
        double const infrate = m_beta * get_infected() / (get_fertile() + get_infertile());
        {
          double const leaveS = m_S.take_rate(infrate, d_time);
          m_I.insert_value_start(leaveS);
        }
        {
          double const leaveSf = m_Sf.take_rate(infrate, d_time);
          m_If.insert_value_start(leaveSf);
        }
      }

      // I(f) -> Af/R(f)
      {
        double const leaveI = m_I.carry_rate(m_ia_rate, d_time);
        double const toR = leaveI * m_screc_prop;
        m_R.insert_value_start(toR);
        m_Af.insert_value_start(leaveI - toR);
      }
      {
        double const leaveIf = m_If.carry_rate(m_ia_rate, d_time);
        double const toRf = leaveIf * m_screc_prop;
        m_Rf.insert_value_start(toRf);
        m_Af.insert_value_start(leaveIf - toRf);
      }

      // Af -> Cf
      // TODO: checl m_disrec_prop is 0!
      m_Cf.insert_value_start( m_Af.carry_rate(m_ac_rate, d_time) );

      update_apply();

    }

    auto update_passive(double const d_time)
      noexcept(!CTS.debug)
      -> void
    {
      if (m_passive_rate > 0.0)
      {
        double const prop = 1.0 - std::exp(-m_passive_rate * d_time);
        treat_vacc_all(prop, s_passive_cull_positive, s_passive_cull_acute, s_passive_cull_chronic);

        update_apply();
      }
    }

    template <typename SrcT, typename DstT>
    auto treat_vacc_noninf(SrcT& src, DstT& dst, double const prop, double const efficacy) noexcept(!CTS.debug)
      -> void
    {
      // Move to or restart in V(f) or R(f):
      dst.insert_value_start( src.take_prop(efficacy * prop) );
    
      // Test positives are treated+vaccinated rather than just vaccinated, but there is no other difference:
      double const test = src.get_sum() * prop;
      m_sumTx += (1.0 - m_sp) * test;
      m_sumVx += m_sp * test;
    }

    enum class Status { nonshedding, subclinical, diseased };

    template <Status s_status, typename Src, typename DstF, typename DstN, typename DstR>
    auto treat_vacc_inf(Src& src, DstF& dstF, DstN& dstN, DstR& dstR,
                        double const prop, double const cull_prob,
                        double const cure_prob) noexcept(!CTS.debug)
      -> void
    {
      double const total_n = src.get_sum();

      double progressed = prop;

      // First round of testing - perfect for Af/Cf, se for I/If, 1-sp for N/Nf:
      double const testpos = (s_status==Status::diseased ? 1.0 : (s_status==Status::subclinical ? m_se : (1.0-m_sp)));
      // All animals that are test negative are vaccinated, but this only affects non-shedding:
      double const to_vacc = progressed * (1.0-testpos);
      m_sumVx += (total_n * to_vacc);
      if constexpr (s_status==Status::nonshedding)
      {
        dstN.insert_value_start(src.take_prop(m_vx_bst * to_vacc));
      }

      progressed *= testpos;

      // A proportion of test-positive animals are culled:
      double const to_cull = progressed * cull_prob;
      {
        double const n_culled = src.take_prop(to_cull);
        m_Z += n_culled;
        m_sumRx += n_culled;
      }

      progressed *= (1.0 - cull_prob);

      // All animals remaining end up being treated:
      m_sumTx += (total_n * progressed);

      // Most have a complete treatment course but some are released early due to a false negative:
      double const treat_complete = (s_status==Status::nonshedding ? 1.0 : m_se); // Note: deliberately not 1-sp here for N!!!
      double const to_frel = progressed * (1.0-treat_complete);
      if constexpr (s_status==Status::diseased)
      {
        // Move diseased to start of I (failed treatment, but no longer clinically diseased)
        dstF.insert_value_start(src.take_prop(to_frel));
      }
      else if constexpr (s_status==Status::subclinical)
      {
        // Do nothing - don't restart animals in I
      }
      else if constexpr (s_status==Status::nonshedding)
      {
        // Restart non-shedding due to vaccine (this will have no effect as test sensitivity above is 1)
        dstN.insert_value_start(src.take_prop(m_vx_bst * to_frel));
      }

      progressed *= treat_complete;

      // Of the complete treatment number, a proportion recover and the rest are non-sheding:
      double const to_nshed = progressed * (1.0 - cure_prob);
      if constexpr (s_status==Status::nonshedding) {
        // For N, restart due to vaccine:
        dstN.insert_value_start(src.take_prop(m_vx_bst * to_nshed));
      }else{
        // Otherwise all move to start of N:
        dstN.insert_value_start(src.take_prop(to_nshed));
      }

      // All cured animals go to start of R:
      double const to_cure = progressed * cure_prob;
      dstR.insert_value_start(src.take_prop(to_cure));

      // Final checks:
      if constexpr (CTS.debug)
      {
        if(std::abs(prop - (to_vacc+to_cull+to_frel+to_nshed+to_cure)) > CTS.tol){
          Rcpp::Rcout << prop << " != " << to_vacc << " + " << to_cull << " + " << to_frel << " + " << to_nshed << " + " << to_cure << "\n";
          Rcpp::stop("Logic error in treat_vacc_inf");
        }
      }

    }

    auto treat_vacc_all(double const prop, double const cull_positive,
                        double const cull_acute, double const cull_chronic)
      noexcept(!CTS.debug)
      -> void
    {
      // Susceptible:
      treat_vacc_noninf(m_S, m_V, prop, m_vx_eff);
      treat_vacc_noninf(m_Sf, m_Vf, prop, m_vx_eff);

      // Already vaccinated:
      treat_vacc_noninf(m_V, m_V, prop, m_vx_bst);
      treat_vacc_noninf(m_Vf, m_Vf, prop, m_vx_bst);

      // Already recovered:
      treat_vacc_noninf(m_R, m_R, prop, m_vx_bst);
      treat_vacc_noninf(m_Rf, m_Rf, prop, m_vx_bst);

      // Non-shedding:
      treat_vacc_inf<Status::nonshedding>(m_N, m_N, m_N, m_R, prop, cull_positive, m_cure_N);
      treat_vacc_inf<Status::nonshedding>(m_Nf, m_Nf, m_Nf, m_Rf, prop, cull_positive, m_cure_N);

      // Infectious:
      treat_vacc_inf<Status::subclinical>(m_I, m_I, m_N, m_R, prop, cull_positive, m_cure_I);
      treat_vacc_inf<Status::subclinical>(m_If, m_If, m_Nf, m_Rf, prop, cull_positive, m_cure_I);

      // Diseased:
      treat_vacc_inf<Status::diseased>(m_Af, m_If, m_Nf, m_Rf, prop, cull_acute, m_cure_A);
      treat_vacc_inf<Status::diseased>(m_Cf, m_If, m_Nf, m_Rf, prop, cull_chronic, m_cure_C);

      update_apply();
    }

    auto update_apply() noexcept(!CTS.debug)
      -> void
    {
      m_S.apply_changes();
      m_V.apply_changes();
      m_N.apply_changes();
      m_I.apply_changes();
      m_R.apply_changes();

      m_Af.apply_changes();
      m_Cf.apply_changes();

      m_Sf.apply_changes();
      m_Vf.apply_changes();
      m_Nf.apply_changes();
      m_If.apply_changes();
      m_Rf.apply_changes();

      check_state();
    }

    [[nodiscard]] auto get_infected()
      const noexcept(!CTS.debug)
      -> double
    {
      double const infected = m_I + m_If + m_Af + m_Cf;
      return infected;
    }

    [[nodiscard]] auto get_fertile()
      const noexcept(!CTS.debug)
      -> double
    {
      double const fertile = m_S + m_V + m_I + m_N + m_R;
      return fertile;
    }

    [[nodiscard]] auto get_infertile()
      const noexcept(!CTS.debug)
      -> double
    {
      double const infertile = m_Af + m_Cf + m_Sf + m_Vf + m_If + m_Nf + m_Rf;
      return infertile;
    }

    auto check_state()
      const noexcept(!CTS.debug)
      -> void
    {
      if constexpr (CTS.debug)
      {
        if(m_S.get_sum() < 0.0) Rcpp::stop("Negative value in S");
        if(m_V.get_sum() < 0.0) Rcpp::stop("Negative value in V");
        if(m_I.get_sum() < 0.0) Rcpp::stop("Negative value in I");
        if(m_N.get_sum() < 0.0) Rcpp::stop("Negative value in N");
        if(m_R.get_sum() < 0.0) Rcpp::stop("Negative value in R");

        if(m_Af.get_sum() < 0.0) Rcpp::stop("Negative value in Af");
        if(m_Cf.get_sum() < 0.0) Rcpp::stop("Negative value in Cf");

        if(m_Sf.get_sum() < 0.0) Rcpp::stop("Negative value in Sf");
        if(m_Vf.get_sum() < 0.0) Rcpp::stop("Negative value in Vf");
        if(m_If.get_sum() < 0.0) Rcpp::stop("Negative value in If");
        if(m_Nf.get_sum() < 0.0) Rcpp::stop("Negative value in Nf");
        if(m_Rf.get_sum() < 0.0) Rcpp::stop("Negative value in Rf");

        double const N = get_fertile() + get_infertile();
        if(std::abs(N + m_Z) > CTS.tol){
          Rcpp::Rcout << -m_Z << " != " << get_fertile() << " + " << get_infertile() << "\n";
          Rcpp::stop("-Z != Fertile + Infertile");
        }
      }
    }


  public:
    KoalaGroup(Rcpp::IntegerVector ncomps, Rcpp::NumericVector const parameters, Rcpp::NumericVector const state) noexcept(!CTS.debug)
      : m_S(s_nS), m_V(ncomps["V"]), m_I(ncomps["I"]), m_N(ncomps["N"]), m_R(ncomps["R"]),
        m_Af(ncomps["A"]), m_Cf(s_nC),
        m_Sf(s_nS), m_Vf(ncomps["V"]), m_If(ncomps["I"]), m_Nf(ncomps["N"]), m_Rf(ncomps["R"])
    {
      set_pars(parameters);
      set_state(state);
    }

    auto set_pars(Rcpp::NumericVector const parameters) noexcept(!CTS.debug)
      -> void
    {
      m_vs_rate = to_rate(parameters["vacc_immune_duration"]);     // #1
      m_ni_rate = to_rate(parameters["vacc_redshed_duration"]);    // #2
      m_rs_rate = to_rate(parameters["natural_immune_duration"]);  // #3
      m_beta = parameters["beta"]/365.0;                           // #4
      m_ia_rate = to_rate(parameters["subcinical_duration"]);      // #5
      m_screc_prop = parameters["subclinical_recover_proportion"]; // #6
      m_disrec_prop = parameters["diseased_recover_proportion"];   // #7
      m_birthrate = parameters["birthrate"]/365.0;                 // #8
      m_ac_rate = to_rate(parameters["acute_duration"]);           // #9
      m_mort_nat = to_rate(parameters["lifespan_natural"]);        // #10
      m_mort_acute = to_rate(parameters["lifespan_acute"]);        // #11
      m_mort_chronic = to_rate(parameters["lifespan_chronic"]);    // #11
      m_rel_fecundity = parameters["relative_fecundity"];          // #12

      m_se = parameters["sensitivity"];
      m_sp = parameters["specificity"];
      m_cure_N = parameters["cure_prob_N"];
      m_cure_I = parameters["cure_prob_I"];
      m_cure_A = parameters["cure_prob_A"];
      m_cure_C = parameters["cure_prob_C"];
      m_vx_eff = parameters["vaccine_efficacy"];
      m_vx_bst = parameters["vaccine_booster"];
      m_passive_rate = parameters["passive_intervention_rate"] / 365.0;
          // -std::log(1.0 - parameters["passive_proportion"]) / 365.0

    }

    [[nodiscard]] auto get_pars() const noexcept(!CTS.debug)
      -> Rcpp::NumericVector
    {
      using namespace Rcpp;
      NumericVector pars = NumericVector::create(
        _["vacc_immune_duration"] = to_duration(m_vs_rate),     // #1
        _["vacc_redshed_duration"] = to_duration(m_ni_rate),    // #2
        _["natural_immune_duration"] = to_duration(m_rs_rate),  // #3
        _["beta"] = m_beta*365.0,                               // #4
        _["subcinical_duration"] = to_duration(m_ia_rate),      // #5
        _["subclinical_recover_proportion"] = m_screc_prop,     // #6
        _["diseased_recover_proportion"] = m_disrec_prop,       // #7
        _["birthrate"] = m_birthrate*365.0,                     // #8
        _["acute_duration"] = to_duration(m_ac_rate),           // #9
        _["lifespan_natural"] = to_duration(m_mort_nat),        // #10
        _["lifespan_acute"] = to_duration(m_mort_acute),        // #11
        _["lifespan_chronic"] = to_duration(m_mort_chronic),    // #11
        _["relative_fecundity"] = m_rel_fecundity,              // #12

        _["sensitivity"] = m_se,
        _["specificity"] = m_sp,
        _["cure_prob_N"] = m_cure_N,
        _["cure_prob_I"] = m_cure_I,
        _["cure_prob_A"] = m_cure_A,
        _["cure_prob_C"] = m_cure_C
      ); // Max number of elements is 20

      pars.push_back(
        m_vx_eff,
        "vaccine_efficacy"
      );

      pars.push_back(
        m_vx_bst,
        "vaccine_booster"
      );

      pars.push_back(
        m_passive_rate*365.0,  // 1.0 - std::exp(-m_passive_rate * 365.0),
        "passive_intervention_rate"
      );

      return pars;
    }

    auto set_state(Rcpp::NumericVector const state) noexcept(!CTS.debug)
      -> void
    {
      m_S.set_sum(state["S"]);
      m_V.set_sum(state["V"]);
      m_I.set_sum(state["I"]);
      m_N.set_sum(state["N"]);
      m_R.set_sum(state["R"]);
      m_Af.set_sum(state["Af"]);
      m_Cf.set_sum(state["Cf"]);
      m_Sf.set_sum(state["Sf"]);
      m_Vf.set_sum(state["Vf"]);
      m_If.set_sum(state["If"]);
      m_Nf.set_sum(state["Nf"]);
      m_Rf.set_sum(state["Rf"]);

      m_day = static_cast<int>(state["Day"]);
      m_sumTx = state["SumTx"];
      m_sumVx = state["SumVx"];
      m_sumRx = state["SumRx"];
      m_sumMx = state["SumMx"];

      m_Z = -(m_S+m_V+m_I+m_N+m_R+m_Af+m_Cf+m_Vf+m_If+m_Nf+m_Rf);
      check_state();

    }

    [[nodiscard]] auto get_state() const noexcept(!CTS.debug)
      -> Rcpp::NumericVector
    {
      check_state();
    
      using namespace Rcpp;
      NumericVector rv = NumericVector::create(
        _["Day"] = static_cast<double>(m_day),
        _["S"] = m_S.get_sum(),
        _["V"] = m_V.get_sum(),
        _["I"] = m_I.get_sum(),
        _["N"] = m_N.get_sum(),
        _["R"] = m_R.get_sum(),
        _["Af"] = m_Af.get_sum(),
        _["Cf"] = m_Cf.get_sum(),
        _["Sf"] = m_Sf.get_sum(),
        _["Vf"] = m_Vf.get_sum(),
        _["If"] = m_If.get_sum(),
        _["Nf"] = m_Nf.get_sum(),
        _["Rf"] = m_Rf.get_sum(),
        _["Z"] = m_Z,
        _["SumTx"] = m_sumTx,
        _["SumVx"] = m_sumVx,
        _["SumRx"] = m_sumRx,
        _["SumMx"] = m_sumMx
      );
      return rv;
    }

    auto active_intervention(double const prop, double const cull_positive,
                              double const cull_acute, double const cull_chronic)
      noexcept(!CTS.debug)
      -> void
    {
      check_state();
    
      // If there are no animals alive then give a warning:
      if ( m_Z < 0.0 ) {
        treat_vacc_all(prop, cull_positive, cull_acute, cull_chronic);
      } else {
        Rcpp::warning("Active intervention requested but no animals alive!");
      }
    }

    auto targeted_intervention(double const prop_acute, double const prop_chronic, 
                              double const cull_acute, double const cull_chronic)
      noexcept(!CTS.debug)
      -> void
    {
      check_state();
    
      // If there are no animals alive then give a warning:
      if ( m_Z < 0.0 ) {
      
        // Acute disease:
        treat_vacc_inf<Status::diseased>(m_Af, m_If, m_Nf, m_Rf, prop_acute, cull_acute, m_cure_A);
      
        // Chronic disease:
        treat_vacc_inf<Status::diseased>(m_Cf, m_If, m_Nf, m_Rf, prop_chronic, cull_chronic, m_cure_C);      
      
        update_apply();
      
      } else {
        Rcpp::warning("Targeted intervention requested but no animals alive!");
      }
    }

    auto update(int const days, double const d_time, bool const record) noexcept(!CTS.debug)
      -> Rcpp::List
    {
      if constexpr (CTS.debug)
      {
        if(days <= 0) Rcpp::stop("Invalid days argument");
        if(m_recording && !record) Rcpp::stop("Can't stop recording when already started!");
      }
    
      check_state();

      int const len = (m_recording ? days : days+1);
      // Rcpp::Rcout << "Updating for " << days << " days (" << len << " rows)\n";
      Rcpp::List rv(len);

      int ii = 0;
      if(!m_recording){
        // Reset cumulative counters:
        m_sumTx = 0.0;
        m_sumVx = 0.0;
        m_sumRx = 0.0;
        m_sumMx = 0.0;

        Rcpp::NumericVector state = get_state();
        rv[ii] = state;
        ii++;
      }
      m_recording = record;

      int newdays = 0;
      while (newdays < days)
      {
        // If there are no animals alive then don't update:
        if ( m_Z < 0.0 ){
          update_death(d_time);
          update_birth(d_time);
          update_disease(d_time);
        }

        m_time += d_time;
        if(m_time >= 1.0)
        {
          // d_time is 1.0 as this happens once per day:
          if ( m_Z < 0.0 ) update_passive(1.0);

          m_time -= 1.0;
          m_day++;
          newdays++;

          Rcpp::NumericVector state = get_state();

          if constexpr (CTS.debug){
            if(ii >= rv.size()) Rcpp::stop("Logic error in update");
          }

          rv[ii] = state;
          ii++;
        }

        check_state();
        Rcpp::checkUserInterrupt();
      };

      check_state();

      return rv;
    }

    auto get_vitals() const noexcept(!CTS.debug)
      -> Rcpp::NumericVector
    {
      check_state();

      double const nn = get_fertile() + get_infertile();
      double const prev = (m_I + m_If + m_Af + m_Cf) / nn;
      using namespace Rcpp;
      NumericVector rv = NumericVector::create(
        _["Day"] = static_cast<double>(m_day),
        _["Alive"] = nn,
        _["Prevalence"] = prev
      );

      return rv;
    }

    // Lazy facilitation of cloning from R6 when we aren't sure of the class template parameters:
    auto clone() const noexcept(!CTS.debug)
      -> KoalaGroup<CTS, s_nV, s_nI, s_nN, s_nR, s_nA>
    {
      return KoalaGroup<CTS, s_nV, s_nI, s_nN, s_nR, s_nA>(*this);
    }

  };

} // blofeld

*/

#endif // BLOFELD_SEIDRVMZ_GROUP_H
