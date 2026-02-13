#ifndef BLOFELD_CONTAINER_H
#define BLOFELD_CONTAINER_H

#include <array>
#include <vector>

#include "./compartment_types.h"

namespace blofeld
{
  
  // This should not be used externally as inheriting from std::array etc invites misuse
  // NOTE: this may be evil, but we will never hold the std::array as a pointer...
  
  namespace internal {
    
    // General container class:
    template<typename Value, ContainerType k_cont_type, int k_n>
    class Container
    {
      Container() = delete; //("Failed to match valid container type");
    };

    // Specialisation for disabled:
    template<typename Value>
    class Container<Value, ContainerType::Disabled, 0>
    {      
    public:
      using ReturnType = std::vector<Value>; // void??
      
      Container() = default;
      
      static consteval void fill(Value const) noexcept
      {
        // Do nothing
      }
      
      static constexpr auto validate() noexcept
        -> bool
      {
        return true;
      }
      
    };

    // Specialisation for array is just a std::array
    template<typename Value, int k_n>
    class Container<Value, ContainerType::Array, k_n> : public std::array<Value, k_n>
    {
    public:
      using ReturnType = std::array<Value, k_n>;
      
      Container()
      {
        static_assert(k_n > 0, "Invalid k_n <= 0 for Container<ContainerType::Array>");
        this->fill(static_cast<Value>(0.0));
      }
      
      auto validate() const noexcept
        -> bool
      {
        bool valid = true;
        for (auto const& val : (*this)) valid = valid && val >= static_cast<Value>(0.0);
        return valid;
      }
      
    };

    // Specialisation for inplace_vector:
    template<typename Value, int k_n>
    class Container<Value, ContainerType::InplaceVector, k_n> : public Container<Value, ContainerType::Array, k_n>
    {
    private:
      int m_n = k_n;
      // Note: clang complains without the (unneccessary) const here:
      static constexpr int const& k_max = k_n;
      
    public:
      using ReturnType = std::vector<Value>;
      
      Container()
      {
        static_assert(false, "ContainerType::InplaceVector is not yet implemented");
      }
      
    };

    // Specialisation for vector:
    template<typename Value, int k_n>
    class Container<Value, ContainerType::Vector, k_n> : public std::vector<Value>
    {
    public:
      Container()
      {
        this->resize(static_cast<std::size_t>(k_n)); // Unnecessary: static_cast<Value>(0.0));
      }
      
      auto validate() const noexcept
        -> bool
      {
        bool valid = true;
        for (auto const& val : (*this)) valid = valid && val >= static_cast<Value>(0.0);
        return valid;
      }
      
    };

    // Specialisation for BirthDeath - same as array, except no check for non-negative
    template<typename Value>
    class Container<Value, ContainerType::BirthDeath, 1> : public Container<Value, ContainerType::Array, 1>
    {
    public:
      
      // Trivial - all values are valid (except NaN??)
      static constexpr auto validate() noexcept
        -> bool
      {
        return true;
      }
      
    };
    
  } // namespace internal

} // namespace blofeld

#endif // BLOFELD_CONTAINER_H
