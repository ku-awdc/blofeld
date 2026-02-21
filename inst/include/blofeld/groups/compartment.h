#ifndef BLOFELD_COMPARTMENT_H
#define BLOFELD_COMPARTMENT_H

#include <array>
#include <numeric>
#include <iostream>
#include <typeinfo>
#include <type_traits>
#include <ranges>

#include "./compartment_types.h"
#include "./container.h"

namespace blofeld
{

  namespace internal
  {
    // To allow conditional checking of carry-forward:
    template<typename T, bool s_active>
    struct MaybeEmpty;

    template<typename T>
    struct MaybeEmpty<T, false>
    {
      
    };

    template<typename T>
    struct MaybeEmpty<T, true>
    {
      T value {};
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
    internal::MaybeEmpty<bool, s_cts.debug> m_take_applied { };
    internal::MaybeEmpty<bool, s_cts.debug> m_carry_applied { };

    // Used when size == 0:
    internal::MaybeEmpty<Value, Resizeable<decltype(m_current)> || decltype(m_working){}.size()==0U> m_carry_through { };
    
    constexpr auto setCarryThrough([[maybe_unused]] Value const value) noexcept
      -> bool
    {
      if constexpr (Resizeable<decltype(m_current)>) {
        if (m_current.size() == 0U) {
          m_carry_through.value = value;
          return true;
        }
      } else if constexpr (decltype(m_working){}.size()==0U) {
         m_carry_through.value = value;
         return true;
      } else {
        // Do nothing
      }
      return false;
    }
    
    constexpr auto getCarryThrough() noexcept
      -> Value
    {
      if constexpr (Resizeable<decltype(m_current)>) {
        if (m_current.size() == 0U) return m_carry_through.value;
      } else if constexpr (decltype(m_working){}.size()==0U) {
        return m_carry_through.value;
      } else {
        // Fall through
      }
      return zero();
    }
    
    constexpr auto isDormant() const noexcept
      -> bool
    {
      bool dormant = true;
      
      // std::zip is C++23:  for (const auto [value1, value2] : std::views::zip(vector1, vector2)) 
      for (int i=0; i<std::ssize(m_current); ++i)
      {
        if (m_current[i] != m_working[i]) dormant = false;
      }
      
      // return m_take_applied.value && m_carry_applied.value && dormant;
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
      
      // Fine even if m_current is size-0 vector or array
      for (auto const& val : m_current)
      {
        if (val < static_cast<Value>(0.0)) {
          m_bridge.stop("Logic error: negative compartment value");
        }
      }

    }

    Compartment() = delete;

  public:
    
    /* Constructors */
    
    constexpr explicit Compartment(Bridge& bridge) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      if constexpr (s_cts.debug) {
        m_take_applied.value = true;
        m_carry_applied.value = true;
      }
      validate();
    }
    
    constexpr explicit Compartment(Bridge& bridge, Value const total) noexcept(!s_cts.debug)
      : m_bridge(bridge)
    {
      if constexpr (s_cts.debug) {
        m_take_applied.value = true;
        m_carry_applied.value = true;
      }
      distribute(total);
      validate();
    }
    
    
    /* Utility function to get a correctly-typed zero */
    static constexpr auto zero() noexcept
    {
      return static_cast<Value>(0);
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
          if (!isDormant()) m_bridge.stop("It is not possible to re-size between applying rates and calling applyChanges()");
        }
        
        const Value total = std::accumulate(m_current.begin(), m_current.end(), zero());
        m_current.resize(size);
        distribute(total);
        m_working = m_current;
        
      } else {
        m_bridge.stop("Container is not resizeable");
      }
    }
        
    // Reset to 0:
    constexpr auto reset() noexcept(!s_cts.debug)
      -> void
    {
      validate();
      m_current.reset();
      m_working = m_current;
      validate();
    }
    
    // Add a total to the first subcompartment:
    constexpr auto insert(Value const total, bool apply_changes = false) noexcept(!s_cts.debug)
      -> void
    {
      // total must be >= 0
      if (total < zero()) {
        m_bridge.stop("Invalid total < 0");
      }
      validate();

      // Shortcut if size==0:
      if(setCarryThrough(total)) return;

      m_working[0] += total;
      validate();
    }

    // Add or remove a fixed number evenly/randomly throughout:
    constexpr auto distribute(Value const total, bool apply_changes = false) noexcept(!s_cts.debug)
      -> void
    {
      if constexpr (Resizeable<decltype(m_current)>) {
        if (ssize(m_current) == 0) {
          m_bridge.stop("Unable to distribute values within an inactive compartment");
        }
      } else {
        // To restrictive as this may be within an un-followed runtime path:      
        // static_assert(decltype(m_current){}.size() > 0U, "Unable to distribute values within a disabled compartment");
        if (ssize(m_current) == 0) {
          m_bridge.stop("Unable to distribute values within a disabled compartment");
        }
      }
      
      // total must be >= -current_value
      if (total < -std::accumulate(m_current.begin(), m_current.end(), zero())) {
        m_bridge.stop("Invalid total < -available");
      }
      
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
            // std::tuple_size_v<decltype(m_working)> works for Array but not BirthDeath
            // Fortunately I have defined ssize as static
            return std::array<double, decltype(m_working)::ssize()>();
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
      if (!isDormant()) m_bridge.stop("It is not possible to set values between applying rates and calling applyChanges()");
      if (values.size() != m_current.size()) m_bridge.stop("Size mis-match in provided values");
      if (s_cts.debug) {
        for (auto val : values)
        {
          if (val < zero()) m_bridge.stop("Invalid value < 0");
        }
      }
      
      std::copy(values.begin(), values.end(), m_current.begin());
      m_working = m_current;
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
        rv.resize(m_current.size());
      } else {
        static_assert(ssize(m_current)==ssize(rv), "Logic error in getValues");        
      }
      std::copy(m_current.begin(), m_current.end(), rv.begin());
      return rv;      
    }
    
    // Get total by value:
    constexpr auto getTotal() const
      -> Value
    {
      return std::accumulate(m_current.begin(), m_current.end(), zero());
    }
        
    // Apply changes from taking rates and inserting/distruting etc:
    constexpr auto applyChanges() noexcept(!s_cts.debug)
      -> void
    {
      if constexpr (s_cts.debug) {
        if (m_carry_applied.value) m_bridge.stop("applyChanges called consecutively without carryProp");
      }
      
      m_current = m_working;
      if constexpr (s_cts.debug) {
        m_take_applied.value = true;
        m_carry_applied.value = true;
      }
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
    // TODO: private?
    [[nodiscard]] constexpr auto adjustCarryRate(double const rate) noexcept(!s_cts.debug)
      -> double
    {
      if constexpr (s_cinfo.carry_type == CarryType::Sequential) {
        
        return rate * ssize(m_current);
        
      } else if constexpr (s_cinfo.carry_type == CarryType::Immediate) {

        static_assert(false, "CarryType::Immediate is not yet implemented");
        
      } else if constexpr (s_cinfo.carry_type == CarryType::None) {
        static_assert(false, "Invalid path to call adjustCarryRate with disabled compartment");
        
      } else {
        static_assert(false, "Unhandled CarryType in adjustCarryRate");
      }
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
          std::vector<double>(take_rates.size()+1U);
        } else {
          std::array<double, decltype(take_rates){}.size()+1U>();
        }        
      }();
      std::copy(take_rates.begin(), take_rates.end(), rates.begin());
      rates.back() = adjustCarryRate(carry_rate);
      
      // Convert to proportions:
      auto props = makeProp(rates);
      
      // Pass down all but the carry prop:
      auto takes = takeProp(std::views::take(props, take_rates.size()));
        
      // Then do the carry:
      Value carry = carryProp(props.back());
      
      // Separate for return:
      TakeCarryValues rv;
      if constexpr (Resizeable<C>) rv.take.resize(take_rates.size());
      std::copy(takes.begin(), takes.end(), rv.take.begin());
      rv.carry = carry;
      
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
      
      // Flag that an update is in progress:
      if constexpr (s_cts.debug) m_take_applied.value = false;
            
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
      Value total = zero();
        
      // Flag that an update is in progress:
      if constexpr (s_cts.debug) m_take_applied.value = false;
      
      for (index c=0; c<ssize(m_current); ++c)
      {
        Value const change = [&](){
          if constexpr (s_mtype==ModelType::Deterministic) {
            return m_working[c] * prop;
          } else if constexpr (s_mtype==ModelType::Stochastic) {
            return m_bridge.rbinom(m_working[c], prop);
          } else {
            static_assert(false, "Unrecognised ModelType in takeProp");
          }              
        }();
        m_working[c] -= change;
        total += change;
      }
        
      return total;
    }
    
    
    /* carryRate and carryProp methods */
    
    // Pass down to carryProp:
    [[nodiscard]] constexpr auto carryRate(double const rate) noexcept(!s_cts.debug)
      -> Value
    {
      return carryProp(makeCarryProp(rate));
    }
    
    // Do the work
    [[nodiscard]] constexpr auto carryProp(double const prop) noexcept(!s_cts.debug)
      -> Value
    {
      if constexpr (s_cts.debug) {
        // Check updates have been done since last time:
        if (!m_carry_applied.value) m_bridge.stop("Attempt to call carryProp more than once without applyChanges");
        // Flag that an update is in progress:
        m_carry_applied.value = false;
      }
      
      // TODO: implement isActive method that can be used in all applyRates etc functions
      const Value carried = [&](){

        if constexpr (s_cinfo.carry_type == CarryType::Sequential) {
        
          Value carry = zero();
          for (int i=0; i<ssize(m_working); ++i)
          {
            const Value n = m_working[i];
            m_working[i] += carry;
            
            if constexpr (s_mtype==ModelType::Deterministic) {
              carry = n*prop;
            } else if constexpr (s_mtype==ModelType::Stochastic) {
              carry = m_bridge.rbinom(n, prop);
            } else {
              static_assert(false, "Unrecognised ModelType in carryProp");
            }
            
            m_working[i] -= carry;
          }
          return carry;
        
        } else if constexpr (s_cinfo.carry_type == CarryType::Immediate) {

          static_assert(false, "CarryType::Immediate is not yet implemented");
        
        } else if constexpr (s_cinfo.carry_type == CarryType::None) {
          static_assert(false, "Invalid path to call carryProp with disabled compartment");
        
        } else {
          static_assert(false, "Unhandled CarryType in carryProp");
        }
        
      }();
      
      return carried + getCarryThrough();
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
    
  /*

    

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
    */

    // Note: unusual + overloads return Value
    [[nodiscard]] constexpr auto operator+(Value const sum) const noexcept(!s_cts.debug)
        -> Value
    {
      return sum + getTotal();
    }

    template <auto s_s, ModelType s_m, CompartmentInfo s_c, typename T>
    [[nodiscard]] constexpr auto operator+(Compartment<s_s, s_m, s_c> const& obj) const noexcept(!s_cts.debug)
        -> Value
    {
      return obj.getTotal() + getTotal();
    }

  };
  
  template<typename T>
  concept ArithmeticType = std::is_arithmetic_v<T>;
  
  // TODO: concept for Compartment
  template <auto s_cts, ModelType s_m, CompartmentInfo s_c, ArithmeticType T>
  [[nodiscard]] constexpr auto operator+(T const sum, Compartment<s_cts, s_m, s_c> const& obj)
    -> decltype(Compartment<s_cts, s_m, s_c>::zero())
  {
    auto tobj = obj.getTotal();
    auto tin = symmetric_cast<decltype(tobj)>(sum);
    return tobj + tin;
  }

  template <auto s_cts, ModelType s_m, CompartmentInfo s_c, ArithmeticType T>
  [[nodiscard]] constexpr auto operator+(Compartment<s_cts, s_m, s_c> const& obj, T const sum)
    -> decltype(Compartment<s_cts, s_m, s_c>::zero())
  {
    auto tobj = obj.getTotal();
    auto tin = symmetric_cast<decltype(tobj)>(sum);
    return tobj + tin;
  }

} //blofeld

#endif // BLOFELD_COMPARTMENT_H
