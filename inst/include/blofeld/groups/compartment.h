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


  // Note: methods are marked constexpr but using that requires a constexpr bridge!
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

    using ReturnContainer = std::conditional_t<
      Resizeable<internal::Container<Value, s_cinfo.container_type, s_cinfo.n>>,
      std::vector<Value>,
      std::array<Value, s_cinfo.n>
    >;
    
    internal::Container<Value, s_cinfo.container_type, s_cinfo.n> m_working;
    internal::Container<Value, s_cinfo.container_type, s_cinfo.n> m_current;

    using Bridge = decltype(s_cts)::Bridge;
    Bridge& m_bridge;
    
    // Used for debug only:
    internal::MaybeBool<s_cts.debug> m_carried;
    
    constexpr auto isDormant() const noexcept
      -> bool
    {
      bool dormant = true;
      
      // std::zip is C++23:  for (const auto [value1, value2] : std::views::zip(vector1, vector2)) 
      for (int i=0; i<std::ssize(m_current); ++i)
      {
        if (m_current[i] != m_working[i]) dormant = false;
      }
      
      return dormant;
    }

    constexpr void checkCompartments() const noexcept(!s_cts.debug)
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

    constexpr void validate()
    {
      if constexpr (s_cts.debug) {
        if (m_current.size() != m_working.size()) m_bridge.stop("Logic error: container sizes unequal within compartment");
      }
      
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
    
    /* Constructors */
    
    constexpr explicit Compartment(Bridge& bridge) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      validate();
    }
    
    constexpr explicit Compartment(Bridge& bridge, Value const total) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      distribute(total);
      validate();
    }
    
    
    /* Methods to change contents */
    
    // Resize:
    constexpr auto resize(int const size)
      -> void
    {
      if constexpr (Resizeable<decltype(m_current)>) {
        
        if constexpr (s_cts.debug) {          
          if (size < 0) m_bridge.stop("Illegal container size < 0 (passed to resize)");
          validate();
          if (!isDormant()) m_bridge.stop("It is not possible to re-size between applying rates and calling update()");
        }
        
        const Value total = std::accumulate(m_current.begin(), m_current.end(), static_cast<Value>(0));
        m_current.resize(size);
        distribute(total);
        m_working = m_current;
        
      } else {
        m_bridge.stop("Container is not resizeable");
      }
    }
        
    // Reset to 0:
    constexpr auto zero() noexcept(!s_cts.debug)
      -> void
    {
      validate();
      m_working.zero();
      validate();
    }
    
    // Add a total to the first subcompartment:
    constexpr auto insert(Value const total, bool apply_changes = false) noexcept(!s_cts.debug)
      -> void
    {
      // total must be >= 0
      validate();
      m_working[0] += total;
      validate();
    }

    // Add or remove a fixed number evenly/randomly throughout:
    constexpr auto distribute(Value const total, bool apply_changes = false) noexcept(!s_cts.debug)
      -> void
    {
      // total must be >= -current_value
      validate();
      
      if constexpr (s_mtype==ModelType::Deterministic) {
        for(auto& val : m_working){
          val += total / ssize(m_working);
        }
      } else if constexpr (s_mtype==ModelType::Stochastic) {
        
        auto probs = [&](){
          if constexpr (Resizeable<decltype(m_working)>) {
            return std::vector<double>(m_working.size());
          } else {
            // Note: clang complains that m_working.size() is not constexpr
            return std::array<double, decltype(m_working){}.size()>();
          }
        }();
        for (auto& pp : probs)
        {
          pp = 1.0 / ssize(m_working);
        }
        
        auto const inits = m_bridge.rmultinom(total, probs);
        for (index i=0; i<ssize(inits); ++i)
        {
          m_working[i] += inits[i];
        }
      } else {
        static_assert(false, "Unrecognised ModelType in distribute");
      }
          
      validate();
    }
    
    // Set the compartments directly:
    template <Container C>
    constexpr auto setValues(C const& values)
      -> void
    {
      static_assert(std::same_as<typename C::value_type, Value>, "Type mis-match: container of Value expected for C");
      
      // values.size() must be right, all values must be >=0, Value type must be right, we can't be mid-update
      // TODO
    }
    
    /* // TODO: needs a const ref accessor in m_current
    // Get compartment values as a const ref:
    ReturnContainer const& getValues() const
    {
      return ...
    } */
    
    // Get compartment values as a copy:
    constexpr auto getValues() const
      -> ReturnContainer
    {
      ReturnContainer rv;
      if constexpr (Resizeable<ReturnContainer>) {
        std::copy(m_current.begin(), m_current.end(), std::back_inserter(rv));
      } else {
        static_assert(ssize(m_current)==ssize(rv), "Logic error in getValues");
        for (index i=0; i<ssize(m_current); ++i)
        {
          rv[i] = m_current[i];
        }
      }      
      return rv;      
    }
        
    // Apply changes from taking rates and inserting/distruting etc:
    constexpr auto applyChanges() noexcept(!s_cts.debug)
      -> void
    {
      // TODO: reset in progress variable
      m_current = m_working;
    }
    
    // Required for Rcpp:
    constexpr auto getValuesV() const
      -> std::vector<Value>
    {
      if constexpr (Resizeable<ReturnContainer>) {
        return getValues();
      } else {
        auto val = getValues();
        std::vector<Value> rv;
        rv.resize(val.size());
        for (index i=0; i<ssize(val); ++i)
        {
          rv[i] = val[i];
        }
        return getValues();
      }
    }
    constexpr auto setValuesV(std::vector<Value> const& values)
      -> void
    {
      setValues(values);
    }
    
    
    /* Methods to convert rates to probabilities */
    
    // Template for any container type:
    template <Container C>
    [[nodiscard]] constexpr auto makeProp(C const& rates) noexcept(!s_cts.debug && !Resizeable<C>)
      -> C
    {
      static_assert(std::same_as<typename C::value_type, double>, "Type mis-match: container of double expected for C");
      
      // Adjust competing rates:
      double const sumrates = std::accumulate(rates.begin(), rates.end(), 0.0);
      double const leave = sumrates==0.0 ? 0.0 : (makeProp(sumrates) / sumrates);
      
      /*
      // Note: using ranges would be nice, but doesn't work with std::array:
      auto props = rates | std::ranges::views::transform([=](double const vv){ return vv * leave; });
      C rv(props.begin(), props.end());
      */
      
      // And actually this might be clearer code anyway:
      C prop = rates;
      for (auto& val : prop)
      {
        val = makeProp(val * leave);
      }      
      return prop;
    }
    
    // Forwarding overload for size-1 array (passes by value not const ref):
    [[nodiscard]] constexpr auto makeProp(std::array<double, 1> const rate) noexcept(!s_cts.debug)
      -> std::array<double, 1>
    {      
      return std::array<double, 1> { makeProp(rate[0]) };
    }
    
    // Overload for simple case of a single rate:
    [[nodiscard]] constexpr auto makeProp(double const rate) noexcept(!s_cts.debug)
      -> double
    {
      double const prop = 1.0 - std::exp(-rate);
      return prop;
    }
    
    // Convert an (always single) carry rate to a probability:
    [[nodiscard]] constexpr auto makeCarryProp(double const rate) noexcept(!s_cts.debug)
      -> double
    {
      double const adj_rate = adjustCarryRate(rate);
      return makeProp(adj_rate);
    }
    
    // Take account of the number of sub-compartments and the carry type:
    [[nodiscard]] constexpr auto adjustCarryRate(double const rate) noexcept(!s_cts.debug)
      -> double
    {
      static_assert(false, "TODO");
      
      return 0.0;
    }
    
        
    /* takeRate functions: pass through to takeProp functions */
    
    // Works with array or vector input rates (maybe also Rcpp::NumericVector ??):
    template <Container C>
    [[nodiscard]] constexpr auto takeRate(C const& rates) noexcept(!s_cts.debug && !Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<Value>, std::array<Value, C{}.size()>>
    {
      static_assert(std::same_as<typename C::value_type, double>, "Type mis-match: container of double expected for C");      
      
      auto props = makeProp(rates);      
      return takeProp(props);      
    }
  
    // Forwarding overload for size-1 array (passes by value not const ref):
    [[nodiscard]] constexpr auto takeRate(std::array<double, 1> const rate) noexcept(!s_cts.debug)
      -> std::array<double, 1>
    {      
      return std::array<double, 1> { takeRate(rate[0]) };
    }
    
    // Overload for simple case of a single rate:
    [[nodiscard]] constexpr auto takeRate(double const rate) noexcept(!s_cts.debug)
      -> Value
    {      
      double const prop = makeProp(rate);
      return takeProp(prop);
    }
    
    
    /* takeCarryRate function: convinience wrapper for use from R (so we don't care about efficiency) */
    
    // Struct for return type of takeCarryRate:
    // TODO: make private?
    template <Container C>
    struct TakeCarryValues
    {
      C take = {};
      double carry = 0.0;
    };
    
    // Works with array or vector input rates (maybe also Rcpp::NumericVector ??):
    template <Container C>
    [[nodiscard]] constexpr auto takeCarryRate(C const& take_rates, double const carry_rate) noexcept(!s_cts.debug && !Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, TakeCarryValues<std::vector<Value>>, TakeCarryValues<std::array<Value, C{}.size()>>>
    {
      static_assert(std::same_as<typename C::value_type, double>, "Type mis-match: container of double expected for C");      
      
      // Add the adjusted carry rate to the end of the take rates:
      auto const rates = [&](){
        if constexpr (Resizeable<C>) {
          std::vector<double>(take_rates.size()+1);
        } else {
          std::array<double, take_rates.size()+1>();
        }        
      }();
      for (index i=0; i<ssize(take_rates); ++i)
      {
        rates[i] = take_rates[i];
      }
      rates.back() = adjustCarryRate(carry_rate);
      
      // Convert to proportions:
      auto props = makeProp(rates);
      
      // Pass down:
      auto all = takeProp(props);
        
      // Separate for return:
      TakeCarryValues rv {};
      for (index i=0; i<ssize(rv.take); ++i)
      {
        rv[i] = all[i];
      }
      rv.carry = all.back();
      
      return rv;
    }
  
    // Forwarding overload for simple case of a single rate:
    [[nodiscard]] constexpr auto takeCarryRate(double const take_rate, double const carry_rate) noexcept(!s_cts.debug)
      -> Value
    {      
      return takeCarryRate(std::array<double, 1> { take_rate }, carry_rate);
    }
  
    
    /* takeProp functions: pass through to double overload of takeProp function */
    
    // Works with array or vector input rates (maybe also Rcpp::NumericVector ??):
    template <Container C>
    [[nodiscard]] constexpr auto takeProp(C const& props) noexcept(!s_cts.debug && !Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<Value>, std::array<Value, C{}.size()>>
    {    
      static_assert(std::same_as<typename C::value_type, double>, "Type mis-match: container of double expected for C");
      // Get return type, which will be C<Value>:
      using R = decltype(this->takeProp(props));
      
      // Need to zero-initialise for std::array:
      R rv = [&](){
        if constexpr (Resizeable<R>) {
          return R(props.size(), static_cast<Value>(0.0));
        } else {
          return R {};
        }
      }();
      
      // Sanity check (clang complains if we use rv and props directly):
      if constexpr (!Resizeable<R>) static_assert(R{}.size() == C{}.size());
      validate();
      
      // For deterministic we can just pass the work down:
      if constexpr (s_mtype==ModelType::Deterministic) {
        for (index i=0; i<ssize(rv); ++i)
        {
          rv[i] = takeProp(props[i]);
        }
      // For stochastic we need multinomial:
      } else if constexpr (s_mtype==ModelType::Stochastic) {
        
        auto probs = [&](){
          if constexpr (Resizeable<C>) {
            return std::vector<double>(props.size()+1U);
          } else {
            return std::array<double, C{}.size()+1U>();
          }
        }();
        
        double tp = 0.0;
        for (index i=0; i<ssize(props); ++i)
        {
          tp += props[i];
          probs[i] = props[i];
        }
        probs.back() = 1.0 - tp;
        
        for (auto& comp : m_working)
        {
          auto take = m_bridge.rmultinom(comp, probs);
          for (index i=0; i<ssize(rv); ++i)
          {
            rv[i] += take[i];
          }
          comp = take.back();
        }
        
      } else {
        static_assert(false, "Unrecognised ModelType in takeProp");
      } 
    
      return rv;
    }
    
    // Forwarding overload for size-1 array (passes by value not const ref):
    [[nodiscard]] constexpr auto takeProp(std::array<double, 1> const rate) noexcept(!s_cts.debug)
      -> std::array<double, 1>
    {      
      return std::array<double, 1> { takeProp(rate[0]) };
    }
  
    // Function that actually does the work
    [[nodiscard]] constexpr auto takeProp(double const prop) noexcept(!s_cts.debug)
      -> Value
    {
      Value total = static_cast<Value>(0);
        
      for (index c=0; c<ssize(m_current); ++c)
      {
        Value const change = [&](){
          if constexpr (s_mtype==ModelType::Deterministic) {
            return m_current[c] * prop;
          } else if constexpr (s_mtype==ModelType::Stochastic) {
            return m_bridge.rbinom(m_current[c], prop);
          } else {
            static_assert(false, "Unrecognised ModelType in takeProp");
          }              
        }();
        m_working[c] -= change;
        total += change;
      }
        
      return total;
    }
    
    
    /* Forwarding methods */
    
    constexpr auto begin() noexcept
    {
      return m_current.begin();
    }
    constexpr auto end() noexcept
    {
      return m_current.end();
    }

    constexpr auto begin() const noexcept
    {
      return m_current.begin();
    }
    constexpr auto end() const noexcept
    {
      return m_current.end();
    }

    constexpr auto cbegin() const noexcept
    {
      return m_current.cbegin();
    }
    constexpr auto cend() const noexcept
    {
      return m_current.end();
    }

    [[nodiscard]] constexpr auto size() const noexcept
      -> std::size_t
    {
      return static_cast<std::size_t>(m_current.size());
    }
    
    // Note: is constexpr for Array etc but not Vector/InplaceVector
    [[nodiscard]] constexpr auto isActive() const noexcept
      -> bool
    {
      return m_current.isActive();
    }    
    
  };
  
  /*

    
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
