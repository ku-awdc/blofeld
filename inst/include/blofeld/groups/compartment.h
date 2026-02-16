#ifndef BLOFELD_COMPARTMENT_H
#define BLOFELD_COMPARTMENT_H

#include <array>
#include <numeric>
#include <iostream>
#include <typeinfo>
#include <type_traits>

#include "./compartment_types.h"
#include "./container.h"

namespace blofeld
{

  namespace internal
  {
    // To allow conditional checking of carry-forward:
    template<bool s_active>
    struct MaybeBool;

    template<>
    struct MaybeBool<false>
    {
    };

    // TODO: forward operator==, operator!=, assignment to value ??
    template<>
    struct MaybeBool<true>
    {
      bool value = false;
    };
    
  } // namespace internal


  template <auto s_cts, ModelType s_mtype, CompartmentInfo s_cinfo>
  class Compartment
  {
  private:

    using Value = std::conditional_t<
      s_mtype == ModelType::Deterministic,
      double,
      std::conditional_t<
        s_mtype == ModelType::Stochastic,
        int,
        void
      >
    >;
    static_assert(!std::is_same<Value, void>::value, "Unrecognised ModelType");
    
    internal::Container<Value, s_cinfo.container_type, s_cinfo.n> m_working;
    internal::Container<Value, s_cinfo.container_type, s_cinfo.n> m_current;

    using Bridge = decltype(s_cts)::Bridge;
    Bridge& m_bridge;
    
    // Used for debug only:
    internal::MaybeBool<s_cts.debug> m_carried;

    auto checkCompartments() const noexcept(!s_cts.debug)
      -> void
    {
      /*
      if constexpr (s_ctype.compcont == CompCont::disabled) return;

      if constexpr (!s_cts.debug && s_ctype.n!=0U)
      {
        if (m_ncomps != s_ctype.n) m_bridge.stop("Non-matching number of sub-compartments");
      }
      if constexpr (s_ctype.n <= 0U)
      {
        m_bridge.stop("Negative s_ctype.n are not allowed, and values of 0 are not yet supported");
      }
      */
    }

    void validate()
    {
      
      checkCompartments();
      
      /*
      if constexpr (s_ctype.compcont == CompCont::disabled) return;

      if constexpr (s_mtype == ModelType::deterministic) {
        static_assert(std::is_same<Value, double>::value, "Value should be double for determinstic models");
      } else if constexpr (s_mtype == ModelType::stochastic) {
        static_assert(std::is_same<Value, int>::value, "Value should be int for determinstic models");
      } else {
        static_assert(false, "Unrecognised ModelType");
      }

      // static_assert(s_ctype.compcont == CompCont::array, "Only array is implemented");
      // TODO: validate container type
      if(m_ncomps!=s_ctype.n)
      {
        m_bridge.stop("Number of compartments does not match between constructor and template parameter");
      };
      */
    }

    Compartment() = delete;

  public:
    explicit Compartment(Bridge& bridge) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      validate();
    }
    
  };
  
  /*

    Compartment(Bridge& bridge, Value const total) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      setSum(total);
      validate();
    }

    [[nodiscard]] constexpr auto isActive() const noexcept
      -> bool
    {
      return m_current.isActive();
    }

    [[nodiscard]] constexpr auto size() const noexcept
      -> std::size_t
    {
      return m_current.size();
    }
    
    [[nodiscard]] auto validate() const noexcept
      -> bool
    {
      // TODO: except for BirthDeath
      bool valid = true;
      for (auto const& val : (*this)) valid = valid && val >= static_cast<Value>(0.0);
      return valid;
    }
    
    
    // Only for Vector and InplaceVector:
    void resize(int const n)
    {
      if constexpr (!Resizeable<decltype(m_current)>) {
        // ERROR
      }
      // TODO: checks first (also for n >= 0 and n <= max for InplaceVector)
      // if constexpr (!)
    }

    // Get and set total:
    [[nodiscard]] auto getTotal() const noexcept
      -> Value
    {
      return std::accumulate(m_current.begin(), m_current.end(), static_cast<Value>(0.0));
    }
    void setTotal(Value const total)
    {
      // TODO
    }

    // Required for this to work as a container for printing as well as getting/setting sub-compartments:
    auto begin() noexcept
    {
      return m_current.begin();
    }
    auto end() noexcept
    {
      return m_current.end();
    }

    auto begin() const noexcept
    {
      return m_current.begin();
    }
    auto end() const noexcept
    {
      return m_current.end();
    }

    auto cbegin() const noexcept
    {
      return m_current.cbegin();
    }
    auto cend() const noexcept
    {
      return m_current.end();
    }


    // Remove from all subcompartments:
    // TODO: constrain T to be a std::array or std::vector
    template <Container T>
    T removeProportion(T props)
    {
      
    }
    Value removeRate(double rate)
    {
      if constexpr (!Resizeable<T> && props.ssize()==1) {
        Value prop = rateToProp(1.0 - std::exp(-rate));
        return removeProportion(prop);
      }
    }

    auto apply_changes()
      noexcept(!s_cts.debug)
      -> void
    {
      / *
      m_bridge.println("{}", m_ctr.get());
      
      if constexpr (s_ctype.compcont == CompCont::disabled) return;

      for(int i=0; i<s_ctype.n; ++i)
      {
        m_values[i] += m_changes[i];
        m_changes[i] = s_zero;

        if constexpr (s_mtype==ModelType::deterministic)
        {
          // Zap occasional small negative values (no adjustment to m_Z, so it can't happen too often):
          if(m_values[i] < s_zero && std::abs(m_values[i]) < s_cts.tol){
            m_values[i] = s_zero;
          }
        }

        if constexpr (s_cts.debug){
          if(m_values[i] < s_zero)
          {
            m_bridge.stop("Applying changes caused a negative value");
          }
        }
      }

      // m_changes has an extra value:
      m_changes[m_changes.size()-1] = s_zero;
      * /
    }
    
    [[nodiscard]] auto get_values() const
      -> ContainerType 
    {
      return m_values;
    }

    [[nodiscard]] auto get_sum() const noexcept(!s_cts.debug)
      -> Value
    {
      if constexpr (s_ctype.compcont == CompCont::disabled) return 0;

      double const rv = std::accumulate(m_values.begin(), m_values.end(), s_zero);
      return rv;
    }

    void set_sum(Value const value, bool const distribute = true) noexcept(!s_cts.debug)
    {
      if constexpr (s_ctype.compcont == CompCont::disabled) return;

      if (distribute)
      {
        if constexpr (s_mtype==ModelType::deterministic)
        {
          for(auto& val : m_values){
            val = value / s_ctype.n;
          }
        } else if constexpr (s_mtype==ModelType::stochastic)
        {
          std::array<double, s_ctype.n-1> probs {};
          for (auto& pp : probs)
          {
            pp = 1.0 / s_ctype.n;
          }
          std::array<Value, s_ctype.n> const
            inits = m_bridge.rmultinom(value, probs);
          for (int i=0; i<s_ctype.n; ++i)
          {
            m_values[i] = inits[i];
          }
        } else
        {
          static_assert(false, "Unrecognised ModelType in set_sum");
        }

      } else {
        for(auto& val : m_values){
          val = s_zero;
        }
        m_values[0] = value;
      }
    }

    / *
    auto remove_number(double const number) noexcept(!s_cts.debug)
      -> double
    {
      if (number > 0.0)
      {
        double const propr = 1.0 - (number / get_sum());
        if constexpr (s_cts.debug)
        {
          if (propr > 1.0) m_bridge.stop("Negative number supplied to remove from compartment");
          if (propr < 0.0) m_bridge.stop("Attempt to remove too high a number from compartment");
        }
        for(auto& val : m_values){
          val *= propr;
        }
      }
      return number;
    }
    * /

    // Overload to allow a single double value for take_rate
    / *
    [[nodiscard]] auto process_rate(double const carry_rate, double const take_rate)
      -> auto
    {
      return process_rate<1>(carry_rate, std::array<double,1> { take_rate });
    }
    * /

    template<std::size_t s_ntake>
    [[nodiscard]] auto process_rate(double carry_rate, std::array<double, s_ntake> const& take_rate)
      -> auto
    {
      // if constexpr (s_ctype.compcont == CompCont::disabled) return;

      // TODO: check all rates are (not strictly) positive
	  
      carry_rate *= s_ctype.n;

      double const sumrates = std::accumulate(take_rate.begin(), take_rate.end(), carry_rate);
      double const leave = sumrates==0.0 ? 0.0 : ((1.0 - std::exp(-sumrates)) / sumrates);

      double const carry_prop = leave == 0.0 ? 0.0 : (leave * carry_rate);
      std::array<double, s_ntake> take_prop {};
      if (leave>0)
      {
        for (int i=0; i<s_ntake; ++i)
        {
          take_prop[i] = leave * take_rate[i];
        }
      }

      return process_prop(carry_prop, take_prop);

    }

    template<std::size_t s_ntake>
    [[nodiscard]] auto process_prop(double const carry_prop, std::array<double, s_ntake> const& take_prop)
      -> auto
    {
      // if constexpr (s_ctype.compcont == CompCont::disabled) return;

      // TODO: check all props are >=0 and sum to <=1
      // TODO: implement take_prop
	  // TODO: this is NOT accounting for n, as process_rate has already done that

      std::array<Value, s_ntake> taken {};

      if constexpr (s_mtype==ModelType::deterministic)
      {
        for (int i=0; i<s_ctype.n; ++i)
        {
          {
            double const tt = m_values[i] * carry_prop;
            m_changes[i] -= tt;
            m_changes[i+1] += tt;
          }
          for (int t=0; t<s_ntake; ++t)
          {
            double const tt = m_values[i] * take_prop[t];
            m_changes[i] -= tt;
            taken[t] += tt;
          }
        }

      } else if constexpr (s_mtype==ModelType::stochastic)
      {

        std::array<double, s_ntake+1> probs;
        probs[0] = carry_prop;
        for (int i=0; i<s_ntake; ++i)
        {
          probs[i+1] = take_prop[i];
        }

        for (int i=0; i<s_ctype.n; ++i)
        {
          std::array<Value, s_ntake+2> const
            cc = m_bridge.rmultinom(m_values[i], probs);
          static_assert(cc.size() == taken.size()+2);

          m_changes[i] -= (m_values[i]-cc[0]);
          m_changes[i+1] += cc[1];
          for (int t=0; t<taken.size(); ++t)
          {
            taken[t] += cc[t+2];
          }
        }

      } else
      {
        static_assert(false, "Unrecognised ModelType in apply_changes");
      }

      if constexpr (s_cts.debug){
        for (auto val : m_values)
        {
          if(val < static_cast<Value>(0))
          {
            m_bridge.stop("Applying changes caused a negative value");
          }
        }
      }

      // TODO: make this a concrete type with bounds-checked accessor for take
      struct
      {
        Value carry;
        std::array<Value, s_ntake> take;
      } rv { m_changes[m_changes.size()-1], taken };

      return rv;
    }

    / *
    [[nodiscard]] auto take_prop(double const prop) noexcept(!s_cts.debug)
      -> double
    {
      double rv = 0.0;
      for(int i=0; i<s_ctype.n; ++i){
        double const tt = m_values[i] * prop;
        rv += tt;
        m_changes[i] -= tt;
      }
      if constexpr (s_cts.debug){
        if(rv < 0.0) m_bridge.stop("Returning negative value from take_prop");
      }
      return rv;
    }

    [[nodiscard]] auto carry_rate(double const rate, double const d_time)
      noexcept(!s_cts.debug)
      -> double
    {
      double const prop = 1.0 - std::exp(-rate * s_ctype.n * d_time);
      double carry = 0.0;
      for(int i=0; i<s_ctype.n; ++i){
        m_changes[i] += carry;
        carry = m_values[i] * prop;
        m_changes[i] -= carry;
      }
      if constexpr (s_cts.debug){
        if(carry < 0.0) m_bridge.stop("Returning negative value from carry_rate");
      }
      return carry;
    }
    * /

    auto insert_value_start(Value const value)
      noexcept(!s_cts.debug)
      -> void
    {
      if constexpr (s_ctype.compcont == CompCont::disabled) return;

      m_changes[0] += value;
    }

    auto ptr()
      noexcept(!s_cts.debug)
      -> ContainerType&
    {
      // TODO: will need to be a span for inplace_vector
      return m_values;
    }

    auto ptr()
      const noexcept(!s_cts.debug)
      -> ContainerType const&
    {
      // TODO: will need to be a span for inplace_vector
      return m_values;
    }

    // Note: unusual + overloads return Value
    [[nodiscard]] auto operator+(Value const sum)
      const noexcept(!s_cts.debug)
        -> Value
    {
      return sum + get_sum();
    }

    template <auto scts, ModelType sm, CompType sc>
    [[nodiscard]] auto operator+(Compartment<scts, sm, sc> const& obj)
      const noexcept(!s_cts.debug)
        -> Value
    {
      return obj.get_sum() + get_sum();
    }

  };
  
  */
  
  // TODO: concept for Compartment
  template <auto s_cts, ModelType s_m, CompartmentInfo s_c, typename T>
  [[nodiscard]] auto operator+(T const sum, Compartment<s_cts, s_m, s_c> const& obj)
    -> T
  {
    using T2 = decltype(obj)::Value;
    static_assert(std::is_same<T, T2>::value, "Inconsistent Value when summing compartments");

    return sum + obj.get_sum();
  }

  template <auto s_cts, ModelType s_m, CompartmentInfo s_c, typename T>
  [[nodiscard]] auto operator+(Compartment<s_cts, s_m, s_c> const& obj, T const sum)
    -> T
  {
    using T2 = decltype(obj)::Value;
    static_assert(std::is_same<T, T2>::value, "Inconsistent Value when summing compartments");

    return sum + obj.get_sum();
  }

} //blofeld

#endif // BLOFELD_COMPARTMENT_H
