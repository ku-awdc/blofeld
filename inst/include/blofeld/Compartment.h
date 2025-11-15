#ifndef BLOFELD_COMPARTMENT_H
#define BLOFELD_COMPARTMENT_H

#include <array>
#include <numeric>
#include <iostream>
#include <typeinfo>

// 

// TODO: should take/carry be immediate?  So only insert is done on delay?
// TODO: allow s_ctype.n = 0 in which case it is set at run time

namespace blofeld
{
  enum class ModelType
  {
    deterministic,  // Uses double and simple maths
    stochastic      // Uses int and random sampling
  };

  enum class CompCont
  {
    disabled,       // Removed - compiles to nothing
    array,          // Fixed size (including 1, but not zero)
    inplace_vector, // Emulation of c++26 inplace_vector i.e. stack-based, dynamic up to max size
    vector,         // Heap-based, dynamic - s_ctype.n is ignored
    balancing       // Fixed size of 1 and allows negative values (for birth/death) - s_ctype.n is ignored
  };

  // Literal type
  struct CompType
  {
    CompCont const compcont;
    size_t const n;
  };
  
  template <auto s_cts, ModelType s_mtype, CompType s_ctype>
  class Compartment
  {
  private:
    
    using ValueType = std::conditional_t<
      s_mtype == ModelType::deterministic,
      double,
      std::conditional_t<
        s_mtype == ModelType::stochastic,
        int,
        void
      >
    >;
        
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
      
    ContainerType m_values { };
    ContainerType m_changes { };
  
    using Bridge = decltype(s_cts)::Bridge;
    Bridge& m_bridge;
  
    const int m_ncomps;

    auto check_compartments() const noexcept(!s_cts.debug)
      -> void
    {
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
      
      // TODO: validate container type
    }

    Compartment() = delete;
  
  public:
    Compartment(Bridge& bridge, int const ncomps) noexcept(!s_cts.debug)
      : m_bridge(bridge), m_ncomps(ncomps)
    {
      validate();
      check_compartments();
    }

    Compartment(Bridge& bridge, int const ncomps, double const value) noexcept(!s_cts.debug)
      : m_bridge(bridge), m_ncomps(ncomps)
    {
      validate();
      set_sum(value);
      check_compartments();
    }

    auto apply_changes()
      noexcept(!s_cts.debug)
      -> void
    {
      for(int i=0; i<s_ctype.n; ++i)
      {
        m_values[i] += m_changes[i];
        m_changes[i] = 0.0;
      
        // Zap occasional small negative values (no adjustment to m_Z, so it can't happen too often):
        if(m_values[i] < 0.0 && std::abs(m_values[i]) < s_cts.tol){        
          m_values[i] = 0.0;
        }
      
        if constexpr (s_cts.debug){
          if(m_values[i] < 0.0){
            //m_bridge.cout << m_values[i] << " < 0.0\n";
            m_bridge.stop("Applying changes caused a negative value");
          }
        }      
      }
    }

    [[nodiscard]] auto get_sum()
      const noexcept(!s_cts.debug)
      -> ValueType
    {
      double const rv = std::accumulate(m_values.begin(), m_values.end(), static_cast<ValueType>(0.0));
      return rv;
    }

    auto set_sum(double const value) noexcept(!s_cts.debug)
      -> void
    {
      // TOODO: switch to distribute balanced or all in first box
      for(auto& val : m_values){
        // val = value / static_cast<double>(s_ctype.n);
        val = static_cast<ValueType>(0.0);
      }
      m_values[0] = value;
    }

    /*
    auto set_rate(double const rate, std::array<double, s_ndests> const& proportions,
                  std::array<double, s_ndests> const& proportions_final)
      noexcept(!s_cts.debug)
      -> void
    {

    }

    auto set_rate(double const rate, std::array<double, s_ndests> const& proportions) noexcept(!s_cts.debug)
      -> void
    {
      set_rate(rate, proportions, proportions);
    }
    */

    [[nodiscard]] auto take_rate(double const rate, double const d_time) noexcept(!s_cts.debug)
      -> double
    {
      double const prop = 1.0 - std::exp(-rate * s_ctype.n * d_time);
      double const rv = take_prop(prop);
      return rv;
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
      
        
      std::array<double, s_ntake> tt = {};
      
      // TODO: make this a concrete type with bounds-checked accessor for take  
      struct
      {
        double carry;
        std::array<double, s_ntake> take;
      } rv { 0.5, tt };
        
      return rv;
    }

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

    auto insert_value_start(double const value)
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

    // Note: unusual + overloads return double
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
