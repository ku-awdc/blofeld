#ifndef BLOFELD_COMPARTMENT_H
#define BLOFELD_COMPARTMENT_H

#include <array>
#include <numeric>
#include <iostream>
#include <typeinfo>

#include "ModelCompTypes.h"

// 

// TODO: should take/carry be immediate?  So only insert is done on delay?
// TODO: allow s_ctype.n = 0 in which case it is set at run time

namespace blofeld
{
  
  template <auto s_cts, ModelType s_mtype, CompType s_ctype>
  class Compartment
  {
  private:
    
    static_assert(s_ctype.n > 0, "Invalid .n in CompType (must be >0)");
    
    using ValueType = std::conditional_t<
      s_mtype == ModelType::deterministic,
      double,
      std::conditional_t<
        s_mtype == ModelType::stochastic,
        int,
        void
      >
    >;
    
    struct NoChecking
    {
    };
    struct Checking
    {
      ValueType balance = s_zero;
    };
    using CheckType = std::conditional_t<
      s_cts.debug,
      Checking,
      NoChecking
    >;
    CheckType m_check;
        
    // TODO: other container types
    using ContainerType = std::conditional_t<
      s_ctype.compcont == CompCont::array,
      std::array<ValueType, s_ctype.n>,
      std::conditional_t<
        s_ctype.compcont == CompCont::vector,
        std::vector<ValueType>,
        void
      >
    >;

    using ChangesType = std::conditional_t<
      s_ctype.compcont == CompCont::array,
      std::array<ValueType, s_ctype.n+1>,
      std::conditional_t<
        s_ctype.compcont == CompCont::vector,
        std::vector<ValueType>,
        void
      >
    >;
        
    static constexpr ValueType s_zero = static_cast<ValueType>(0);
      
    ContainerType m_values { };
    ChangesType m_changes { };
  
    using Bridge = decltype(s_cts)::Bridge;
    Bridge& m_bridge;
  
    // TODO: only constexpr for certain container types
    static constexpr int m_ncomps = s_ctype.n;

    auto check_compartments() const noexcept(!s_cts.debug)
      -> void
    {
      static_assert(s_ctype.n > 0, "Invalid .n in CompType");
      
      if constexpr (!s_cts.debug && s_ctype.n!=0U)
      {
        if (m_ncomps != s_ctype.n) m_bridge.stop("Non-matching number of sub-compartments");
      }
      if constexpr (s_ctype.n <= 0U)
      {
        m_bridge.stop("Negative s_ctype.n are not allowed, and values of 0 are not yet supported");
      }    
    }

    void validate()
    {
      if constexpr (s_mtype == ModelType::deterministic) {
        static_assert(std::is_same<ValueType, double>::value, "ValueType should be double for determinstic models");
      } else if constexpr (s_mtype == ModelType::stochastic) {
        static_assert(std::is_same<ValueType, int>::value, "ValueType should be int for determinstic models");
      } else {
        static_assert(false, "Unrecognised ModelType");
      }
      
      static_assert(s_ctype.compcont == CompCont::array, "Only array is implemented");
      // TODO: validate container type
      if(m_ncomps!=s_ctype.n)
      {
        m_bridge.stop("Number of compartments does not match between constructor and template parameter");  
      };
    }

    Compartment() = delete;
  
  public:
    explicit Compartment(Bridge& bridge) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      validate();
      check_compartments();
    }

    Compartment(Bridge& bridge, double const value) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      validate();
      set_sum(value);
      check_compartments();
    }
    
    auto size() const
      -> std::size_t
    {
      return s_ctype.n;
    }
    
    auto begin() noexcept
    {
      return m_values.begin();
    }
    auto end() noexcept
    {
      return m_values.end();
    }

    auto begin() const noexcept
    {
      return m_values.begin();
    }
    auto end() const noexcept
    {
      return m_values.end();
    }
    
    auto cbegin() const noexcept
    {
      return m_values.cbegin();
    }
    auto cend() const noexcept
    {
      return m_values.end();
    }
    
    auto apply_changes()
      noexcept(!s_cts.debug)
      -> void
    {
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
    }

    [[nodiscard]] auto get_sum()
      const noexcept(!s_cts.debug)
      -> ValueType
    {
      double const rv = std::accumulate(m_values.begin(), m_values.end(), s_zero);
      return rv;
    }

    auto set_sum(ValueType const value, bool const distribute = true) noexcept(!s_cts.debug)
      -> void
    {
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
          std::array<ValueType, s_ctype.n> const
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

    /*
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
    */

    // Overload to allow a single double value for take_rate
    /*
    [[nodiscard]] auto process_rate(double const carry_rate, double const take_rate)
      -> auto
    {
      return process_rate<1>(carry_rate, std::array<double,1> { take_rate });        
    }
    */
    
    template<size_t s_ntake>
    [[nodiscard]] auto process_rate(double const carry_rate, std::array<double, s_ntake> const& take_rate)
      -> auto
    {
      // TODO: check all rates are (not strictly) positive
      
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

    template<size_t s_ntake>
    [[nodiscard]] auto process_prop(double const carry_prop, std::array<double, s_ntake> const& take_prop)
      -> auto
    {
      // TODO: check all props are >=0 and sum to <=1
      // TODO: implement take_prop
      
      std::array<ValueType, s_ntake> taken {};
      
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
          std::array<ValueType, s_ntake+2> const
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
          if(val < static_cast<ValueType>(0))
          {
            m_bridge.stop("Applying changes caused a negative value");
          }
        }
      }      
      
      // TODO: make this a concrete type with bounds-checked accessor for take  
      struct
      {
        ValueType carry;
        std::array<ValueType, s_ntake> take;
      } rv { m_changes[m_changes.size()-1], taken };
        
      return rv;
    }

    /*
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
    */

    auto insert_value_start(ValueType const value)
      noexcept(!s_cts.debug)
      -> void
    {
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

    // Note: unusual + overloads return ValueType
    [[nodiscard]] auto operator+(ValueType const sum)
      const noexcept(!s_cts.debug)
        -> ValueType
    {
      return sum + get_sum();
    }
    
    template <auto scts, ModelType sm, CompType sc>
    [[nodiscard]] auto operator+(Compartment<scts, sm, sc> const& obj)
      const noexcept(!s_cts.debug)
        -> ValueType
    {
      return obj.get_sum() + get_sum();
    }
    
  };

  template <auto scts, ModelType sm, CompType sc, typename T>
  [[nodiscard]] auto operator+(T const sum, Compartment<scts, sm, sc> const& obj)
    -> T
  {
    using T2 = decltype(obj)::ValueType;
    static_assert(std::is_same<T, T2>::value, "Inconsistent ValueType when summing compartments");
    
    return sum + obj.get_sum();
  }

  template <auto scts, ModelType sm, CompType sc, typename T>
  [[nodiscard]] auto operator+(Compartment<scts, sm, sc> const& obj, T const sum)
    -> T
  {
    using T2 = decltype(obj)::ValueType;
    static_assert(std::is_same<T, T2>::value, "Inconsistent ValueType when summing compartments");
    
    return sum + obj.get_sum();
  }

} //blofeld

#endif // BLOFELD_COMPARTMENT_H
