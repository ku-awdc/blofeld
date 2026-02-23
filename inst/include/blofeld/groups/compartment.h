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
#include "../utilities/identical.h"

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
    internal::MaybeEmpty<Value, Resizeable<decltype(m_current)> || decltype(m_working){}.empty()> m_carry_through { };
    
    [[nodiscard]] constexpr auto setCarryThrough([[maybe_unused]] Value const value) noexcept
      -> bool
    {
      if constexpr (Resizeable<decltype(m_current)>) {
        if (m_current.size() == 0U) {
          m_carry_through.value = value;
          return true;
        }
      } else if constexpr (decltype(m_working){}.empty()) {
         m_carry_through.value = value;
         return true;
      } else {
        // Do nothing
      }
      return false;
    }
    
    [[nodiscard]] constexpr auto getCarryThrough() noexcept
      -> Value
    {
      if constexpr (Resizeable<decltype(m_current)>) {
        if (m_current.size() == 0U) return m_carry_through.value;
      } else if constexpr (decltype(m_working){}.empty()) {
        return m_carry_through.value;
      } else {
        // Fall through
      }
      return zero();
    }
    
    [[nodiscard]] constexpr auto isDormant() const noexcept
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

    constexpr auto checkCompartments() const noexcept(!s_cts.debug)
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
      
      // Fine even if m_current is size-0 vector or array
      for (auto const& val : m_current)
      {
        if (val < static_cast<Value>(0.0)) {
          m_bridge.stop("Logic error: negative compartment value");
        }
      }
      
    }

    constexpr auto validate()
      -> void
    {
      if constexpr (s_cts.debug) {
        if (m_current.size() != m_working.size()) m_bridge.stop("Logic error: container sizes unequal within compartment");
      }
      
      checkCompartments();
      
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
    [[nodiscard]] static constexpr auto zero() noexcept
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
    [[nodiscard]] constexpr auto getValues() const
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
    [[nodiscard]] constexpr auto getTotal() const
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
    [[nodiscard]] constexpr auto getValuesV() const
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
    
    
    /* Convinience forwarding methods */
    /* EFFICIENCY CONCERNS
    // godbolt.org strongly suggests that passing a size-0 array by (const) ref is the same as by value i.e. no instructions omitted
    // Also it looks like passing a size-1 array by (const) ref is the same as by value is the same as the value of arr.front()
    // Passing a size-2+ array by const ref is cheaper than by value, as expected
    // Conclusion: passing through to takeCarryProps using std::array of size 0 or 1 should be efficient for specialist calls
    */    
    
    // Make a single take proportion from rate:
    [[nodiscard]] constexpr auto makeTakeProp(double const take_rate) noexcept(!s_cts.debug)
      -> double
    {
      auto [take, _] = makeProps(std::array<double, 1> { take_rate }, std::array<double, 0> {});
      return take;
    }

    // Make a single carry proportion from rate:
    [[nodiscard]] constexpr auto makeCarryProp(double const carry_rate) noexcept(!s_cts.debug)
      -> double
    {
      auto [_, carry] = makeProps(std::array<double, 0> { }, std::array<double, 1> { carry_rate });
      return carry;
    }
    
    // Take rates only:
    template <Container C>
    [[nodiscard]] constexpr auto takeRate(C const& take_rate) noexcept(!s_cts.debug && !Resizeable<C>)
      -> std::conditional_t<Resizeable<C>, std::vector<Value>, std::array<Value, C{}.size()>>
    {
      auto [take, _] = takeCarryRates(take_rate, std::array<double, 0> {});     
      return take;  
    }
  
    // Take a single rate only:
    [[nodiscard]] constexpr auto takeRate(double const take_rate) noexcept(!s_cts.debug)
      -> Value
    {
      auto [take, _] = takeCarryRates(std::array<double, 1> { take_rate }, std::array<double, 0> {});
      return take.front();
    }
    
    // Carry (always a single) rate only:
    [[nodiscard]] constexpr auto carryRate(double const carry_rate) noexcept(!s_cts.debug)
      -> Value
    {
      auto [_, carry] = takeCarryRates(std::array<double, 0> {}, std::array<double, 1> { carry_rate });
      return carry.front();
    }
  
    // takeCarryRates to call makeProps then takeCarryProps:
    template <Container C, std::size_t s_nc>
    [[nodiscard]] constexpr auto takeCarryRates(
      C const& take_rate,                         // Any container, including size-0
      std::array<double, s_nc> const carry_rate   // Either size-0 or size-1 (and therefore pass by value)
    ) noexcept(s_cts.debug)
    {
      auto [take_props, carry_props] = makeProps(take_rate, carry_rate);
      return takeCarryProps(take_props, carry_props);
    }

    
    /* Underlying functions that do the work (even when we have only a single proportion) */

    // makeProps to convert rates into proportions, inculding adjustment of carry_rate where needed:
    template <Container C, std::size_t s_nc>
    [[nodiscard]] constexpr auto makeProps(
      C const& take_rate,                         // Any container, including size-0
      std::array<double, s_nc> const carry_rate   // Either size-0 or size-1 (and therefore pass by value)
    ) noexcept(s_cts.debug)
    {
      // Pre-conditions:
      static_assert(std::same_as<typename C::value_type, double>, "Invalid arguments to makeProps:  container of double expected for C");      
      static_assert(s_nc <= 1U, "Invalid arguments to makeProps: invalid std::array<double, 2+> passed as carry_rate");
      
      static_assert(s_nc==0U || s_nc==1U, "Logic error in makeProps:  s_nc not in {0,1}");
      constexpr bool s_carry = s_cinfo.carry_type!=CarryType::None && s_nc!=0U;
      // Note: if CarryType::None then simply ignore any provided carry_rate and convert to 0
      // \Pre-conditions
      
      // Adjust competing rates (accumulate works with size-0 arrays):
      double const carry_adj = [&](){
        if constexpr (!s_carry || s_cinfo.carry_type == CarryType::None) {
          return 0.0;
        } else if constexpr (s_cinfo.carry_type == CarryType::Sequential) {
          return carry_rate.front() * static_cast<double>(ssize(m_working));
        } else if constexpr (s_cinfo.carry_type == CarryType::Immediate) {
          static_assert(false, "Logic error in makeProps: CarryType::Immediate is not yet implemented");
          return carry_rate.front() * static_cast<double>(ssize(m_working));
        } else {
          static_assert(false, "Logic error in makeProps: unhandled CarryType");
        }
      }();
      double const sumrates = std::accumulate(take_rate.begin(), take_rate.end(), carry_adj);
      double const adj = sumrates==0.0 ? 0.0 : ((1.0 - std::exp(-sumrates)) / sumrates);
      
      // Re-usable lambda:
      auto rateToProp = [adj](double const rate) {
        return 1.0 - std::exp(-rate * adj);
      };
          
      // Return values:
      auto rv = [&](){
        if constexpr (Fixedsize<C> && C{}.size()==0U) {
          if constexpr (s_carry) {
            // Only carry needed:
            struct {
              std::array<double, 0> take_prop;
              std::array<double, 1> carry_prop;
            } tt {};
            return tt;
          } else {
            // Nothing needed:
            struct {
              std::array<double, 0> take_prop;
              std::array<double, 0> carry_prop;
            } tt {};
            return tt;
          }
        } else {
          using R = std::conditional_t<
            Resizeable<C>,
            std::vector<double>,
            std::array<double, C{}.size()>
          >;
          if constexpr (s_carry) {
            // Both take and carry needed:
            struct {
              R take_prop {};
              std::array<double, 1> carry_prop;
            } tt {};
            if constexpr (Resizeable<C>) tt.take_prop.resize(take_rate.size());
            return tt;
          } else {
            // Only take needed:
            struct {
              R take_prop {};
              std::array<double, 0> carry_prop;
            } tt {};
            if constexpr (Resizeable<C>) tt.take_prop.resize(take_rate.size());
            return tt;
          }
        } 
      }();
      
      // Update the values:
      if constexpr (Resizeable<C> || decltype(take_rate){}.size() > 0U) {
        if constexpr (s_cts.debug) {
          if (!identical(ssize(take_rate), ssize(rv.take_prop))) m_bridge.stop("Logic error in makeProps: unequal length take_rate and take_prop");
        }
        for (index i=0; i<ssize(take_rate); ++i)
        {
          rv.take_prop[i] = rateToProp(take_rate[i]);
        }
      }
      if constexpr (s_carry) rv.carry_prop.front() = rateToProp(carry_adj);

      return rv;  
    }

    // takeCarryProps to do the actual work:
    template <Container C, std::size_t s_nc>
    [[nodiscard]] constexpr auto takeCarryProps(
      [[maybe_unused]] C const& take_prop,                        // Any container, including size-0 - ignored if inactive
      [[maybe_unused]] std::array<double, s_nc> const carry_prop  // Either size-0 or size-1 (and therefore pass by value) - ignored if inactive or CarryType::None
    ) noexcept(s_cts.debug)
    {
      // Pre-conditions:
      static_assert(std::same_as<typename C::value_type, double>, "Invalid arguments to takeCarryProps:  container of double expected for C");      
      static_assert(s_nc <= 1U, "Invalid arguments to takeCarryProps: iInvalid std::array<double, 2+> passed as carry_prop");
      
      static_assert(s_nc==0U || s_nc==1U, "Logic error in takeCarryProps:  s_nc not in {0,1}");      
      constexpr bool s_carry = s_cinfo.carry_type!=CarryType::None && s_nc!=0U;
      // Note: if CarryType::None then simply ignore any provided carry_prop
      
      if constexpr (s_cts.debug) {
        double sumprop = std::accumulate(take_prop.begin(), take_prop.end(), 0.0);
        if constexpr (s_carry) sumprop += carry_prop[0];
        if (sumprop > 1.0) m_bridge.stop("Invalid arguments to takeCarryProps:  sum of props exceeds 1");
      }
      // \Pre-conditions
      
      // Flag that an update is in progress:
      if constexpr (s_cts.debug) {
        m_take_applied.value = false;
        
        // Check carry rates are applied exactly once:
        if constexpr (s_carry) {
          if (!m_carry_applied.value) m_bridge.stop("Runtime error: attempt to call carryProp more than once without applyChanges");
          m_carry_applied.value = false;
          /* TODO: uncomment below and update applyChanges to match
          if constexpr (Fixedsize<C> && !C{}.empty()) {
            m_carry_applied.value = false;
          } else if constexpr (Resizeable<C>) {
            if (!m_working.empty()) m_carry_applied.value = false;
          }
          */
        }
      }
      
      // Set up different return structs depending on what we are going to need:
      auto rv = [&](){
        if constexpr (Fixedsize<C> && C{}.size()==0U) {
          if constexpr (s_carry) {
            // Only carry needed:
            struct {
              std::array<Value, 0> take;
              std::array<Value, 1> carry = { zero() };
            } tt;
            return tt;
          } else {
            // Nothing needed:
            struct {
              std::array<Value, 0> take;
              std::array<Value, 0> carry;
            } tt;
            return tt;
          }
        } else {
          using R = std::conditional_t<
            Resizeable<C>,
            std::vector<Value>,
            std::array<Value, C{}.size()>
          >;
          if constexpr (s_carry) {
            // Both take and carry needed:
            struct {
              R take {};
              std::array<Value, 1> carry = { zero() };
            } tt;
            if constexpr (Resizeable<C>) tt.take.resize(take_prop.size());
            return tt;
          } else {
            // Only take needed:
            struct {
              R take {};
              std::array<Value, 0> carry;
            } tt;
            if constexpr (Resizeable<C>) tt.take.resize(take_prop.size());
            return tt;
          }
        } 
      }();
      
      // Sanity checks:
      if constexpr (s_cts.debug && (Resizeable<C> || C{}.size()>0U)) {
        if (rv.take.size() != take_prop.size()) m_bridge.stop("Logic error in takeCarryProps:  rv and take_prop unequal size");
      }
      
      // Short circuit in case we are inactive:
      if constexpr (s_carry && Fixedsize<decltype(m_working)> && decltype(m_working){}.empty()) {
        rv.carry.front() = getCarryThrough();
        return rv;
      } else if constexpr (s_carry && Resizeable<decltype(m_working)>) {
        if (m_working.empty()) {
          rv.carry.front() = getCarryThrough();
          return rv;
        } else {
          // Sanity check:
          if constexpr (s_cts.debug) if (!identical(getCarryThrough(), zero())) m_bridge.stop("Logic error: non-zero carry-through for active compartment");
        }
      } else {
        // Sanity check:
        if constexpr (s_carry && s_cts.debug) if (!identical(getCarryThrough(), zero())) m_bridge.stop("Logic error: non-zero carry-through for active compartment");
      }
      
      // If we have a carry prop then we need to track that:
      auto carry = [](){
        if constexpr (s_carry) {
          struct { double value = zero(); } tt;
          return tt;
        } else {
          struct { } tt;
          return tt;
        }
      }();
      
      // Outer loop is the sub-compartment, as we need to do everything (take and carry) together:
      for (auto& cc : m_working)
      {
        // For Deterministic we just need the total removed, for Stochastic we also need the adjusted probability:
        auto removed = [](){
          if constexpr (s_mtype==ModelType::Deterministic) {
            struct {
              Value value = zero();
            } tt;
            return tt;
          } else if constexpr (s_mtype==ModelType::Stochastic) {
            struct {
              Value value = zero();
              double prop = 1.0;
            } tt;
            return tt;
          } else {
            static_assert(false, "Unhandled ModelType in takeCarryProps");
          }
        }();
        
        // First deal with the take proportion(s) - this could be a size-0 C:
        for (index i=0; i<ssize(take_prop); ++i)
        {
          // Calculate the removal:
          Value tt = [&](){
            if constexpr (s_mtype==ModelType::Deterministic) {
              // For deterministic it is just a fixed proportion:
              return cc*take_prop[i];
              
            } else if constexpr (s_mtype==ModelType::Stochastic) {
              // For stochastic we also need to adjust the probability:
              int const val = m_bridge.rbinom(cc - removed.value, take_prop[i] / removed.prop);
              removed.prop -= take_prop[i];
              return val;
              
            } else {
              static_assert(false, "Logic error in takeCarryProps: unhandled ModelType");
            }
          }();          
          
          // Error and bounds checking:
          if constexpr (s_cts.debug) {
            if (tt < zero()) m_bridge.stop("Logic error in takeCarryProps:  tt < 0");
            if (i >= ssize(rv.take)) m_bridge.stop("Bounds check error in takeCarryProps:  rv.take too small");
          }          
          
          // Changes:
          removed.value += tt;
          rv.take[i] += tt;
        }

        // Then deal with the carry proportion, if there is one:
        if constexpr (s_carry) {
        
          // For CarryType::Immediate we need to apply any carry-forward beforehand:
          if constexpr (s_cinfo.carry_type == CarryType::Immediate) {
            static_assert(false, "NEEDS TESTING");
            cc += carry.value;
          }
        
          // Calculate the carry:
          Value tt = [&](){
            if constexpr (s_mtype==ModelType::Deterministic) {
              // For deterministic it is just a fixed proportion:
              return cc*carry_prop[0];
              
            } else if constexpr (s_mtype==ModelType::Stochastic) {
              // Sanity check:
              if constexpr (s_cts.debug) {
                if (!identical(1.0-removed.prop, std::accumulate(take_prop.begin(), take_prop.end(), 0.0), s_cts.tol)) m_bridge.stop("Logic error in takeCarryProps:  1-removed.prop ({}) != sum(take_prop) ({})", 1.0-removed.prop, std::accumulate(take_prop.begin(), take_prop.end(), 0.0));
              }
              // For stochastic we also need to use the adjusted probability:
              return m_bridge.rbinom(cc - removed.value, carry_prop[0] / removed.prop);
              
            } else {
              static_assert(false, "Logic error in takeCarryProps: unhandled ModelType");
            }
          }();          
        
          // For CarryType::Sequential we need to apply any carry-forward afterward:
          if constexpr (s_cinfo.carry_type == CarryType::Sequential) {
            cc += carry.value;
          }
          
          // Carry forward to next subcompartment:
          cc -= tt;
          carry.value = tt;
          
          static_assert(
            s_cinfo.carry_type == CarryType::Sequential || s_cinfo.carry_type == CarryType::Immediate, 
            "Logic error in takeCarryProps:  unhandled CarryType"        
          );
        
        } else {
          static_assert(s_nc==0U, "Logic error in takeCarryProps:  s_nc not in {1,0}");
        }
        
        // Then finally apply changes to m_working:
        cc -= removed.value;        
      }
      
      // Then add the final carry value:
      if constexpr (s_carry) {
        rv.carry.front() = carry.value;
      }
      
      // Sanity check:
      if constexpr (s_cts.debug) {
        // TODO: have a struct holding the previous total and the running total changes, then
        // when running applyChanges and takeCarryProb ensure this add up
      }
      
      return rv;
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
    [[nodiscard]] constexpr auto empty() const noexcept
      -> bool
    {
      return m_current.empty();
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
