#ifndef BLOFELD_CONTAINER_H
#define BLOFELD_CONTAINER_H

#include <array>
#include <vector>
#include <stdexcept>
#include <concepts>

#include "./compartment_types.h"

namespace blofeld
{
  
  // This should not be used externally as inheriting from std::array etc invites misuse
  // NOTE: this may be evil, but we will never hold the std::array as a pointer...

  // TODO: change inheritence  to composition
  // TODO: implement copy/move constructors assuming/checking lengths are equal and copying elements using std::copy
  
  // TODO: change isActive to empty (consistency with std::vector)
  // TODO: allow std::array<..., 0>
  // TODO: make this more robust:
  /*
    template<template<typename...> class Ref, typename... Args>
    struct is_specialization<Ref<Args...>, Ref>: std::true_type {};
  */
  
  template<typename T>
  concept Container = requires(T x)
  {
    { x.size() } -> std::same_as<std::size_t>;
  };
  
  template<typename T>
  concept Resizeable = Container<T> && requires(T x)
  {
    { x.resize(0) } -> std::same_as<void>;
  };

  template<typename T>
  concept Maxsized = Container<T> && (!Resizeable<T> || requires(T x)
  {
    { x.maxSize() } -> std::convertible_to<int>;
  });

  template<typename T>
  concept Fixedsize = Container<T> && !Resizeable<T>;
  
  
  namespace internal {
    
    template<typename T>
    concept BlofeldContainer = Container<T> && requires(T x)
    {
      { x.isActive() } -> std::same_as<bool>;
      { x.ssize() } -> std::same_as<int>;
    };
    
    // General container class:
    template<typename Value, ContainerType s_cont_type, int s_n>
    class Container
    {
      Container() = delete; //("Failed to match valid container type");
    };

    // TODO: define array first, then Disabled is same as array - define empty() in terms of ssize() (DRY)
    
    // Specialisation for disabled is just a std::array with size 0:
    template<typename Value>
    class Container<Value, ContainerType::Disabled, 0> : public std::array<Value, 0>
    {      
    public:
      using ReturnType = std::array<Value, 0>;
      
      static constexpr void zero() noexcept
      {
        // Do nothing
      }
      
      [[nodiscard]] static constexpr auto ssize() noexcept
        -> int
      {
        return 0;
      }
      
      [[nodiscard]] static constexpr auto isActive() noexcept
        -> bool
      {
        return false;
      }
      
    };

    // Specialisation for array (n==0) is identical to disabled:
    template<typename Value>
    class Container<Value, ContainerType::Array, 0> : public Container<Value, ContainerType::Disabled, 0> {};

    // Specialisation for array (n>0) is just a std::array
    template<typename Value, int s_n>
    class Container<Value, ContainerType::Array, s_n> : public std::array<Value, s_n>
    {
    public:
      using ReturnType = std::array<Value, s_n>;
      
      Container()
      {
        static_assert(s_n > 0, "Invalid s_n <= 0 for Container<ContainerType::Array>");
        zero();
      }
            
      void zero() noexcept
      {
        this->fill(static_cast<Value>(0));
      }
      
      [[nodiscard]] static constexpr auto ssize() noexcept
        -> int
      {
        return s_n;
      }
            
      [[nodiscard]] static constexpr auto isActive() noexcept
        -> bool
      {
        return true;
      }      
      
    };

    // Specialisation for inplace_vector:
    template<typename Value, int s_n>
    class Container<Value, ContainerType::InplaceVector, s_n> : public Container<Value, ContainerType::Array, s_n>
    {
    private:      
      int m_n = s_n;
      // Note: clang complains without the (unneccessary) const here:
      static constexpr int const& s_max = s_n;
      
    public:
      using ReturnType = std::vector<Value>;
      
      Container()
      {
        static_assert(s_n > 0, "Invalid s_n <= 0 for Container<ContainerType::InplaceVector>");
        
        this->zero();
        resize(static_cast<std::size_t>(s_n)); // Unnecessary: static_cast<Value>(0.0));
      }
      
      void resize(int const n)
      {
        if (n > s_max) throw std::invalid_argument("Attempt to set n > maxSize() in Container<ContainerType::InplaceVector>.resize()");
        m_n = n;
      }
      
      [[nodiscard]] auto size() const noexcept
        -> std::size_t
      {
        return static_cast<std::size_t>(m_n);
      }

      [[nodiscard]] auto ssize() const noexcept
        -> int
      {
        return m_n;
      }
      
      [[nodiscard]] static constexpr auto maxSize() noexcept
      {
        return s_max;
      }

      [[nodiscard]] auto isActive() const noexcept
        -> bool
      {
        return (size() > 0U);
      }      
      
      [[nodiscard]] auto empty() const noexcept
        -> bool
      {
        return isActive();
      }
      
      // Temporary until we have C++26 inplace_vector:
      auto end() noexcept
      {
        auto ptr = this->begin();
        return ptr+m_n;
      }
      auto end() const noexcept
      {
        auto ptr = this->cbegin();
        return ptr+m_n;
      }      
      auto cend() const noexcept
      {
        return end();
      }      
      template <typename T>
      void swap(T)
      {
        throw std::logic_error("Unable to call swap on a Container<ContainerType::InplaceVector> (until I change my implementation to C++26 inplace_vector)");
      }      
      void back()
      {
        throw std::logic_error("Unable to call swap on a Container<ContainerType::InplaceVector> (until I change my implementation to C++26 inplace_vector)");
      }
      
    };

    // Specialisation for vector:
    template<typename Value, int s_n>
    class Container<Value, ContainerType::Vector, s_n> : public std::vector<Value>
    {
    public:
      Container()
      {
        resize(s_n);
      }
      
      void zero() noexcept
      {
        for (auto& val : (*this)) val = static_cast<Value>(0);
      }
      
      // Overloading this ensures we can't request a size that's bigger than max int:
      void resize(int const n)
      {
        if (n < 0) throw std::invalid_argument("Attempt to set n < 0 in Container<ContainerType::Vector>.resize()");
        std::vector<Value>::resize(static_cast<std::size_t>(n));  // Default second argument: static_cast<Value>(0.0));        
      }
      
      [[nodiscard]] auto ssize() const noexcept
        -> int
      {
        // Guaranteed to fit into an int as we overloaded resize:
        return static_cast<int>(this->size());
      }
      
      [[nodiscard]] auto isActive() const noexcept
        -> bool
      {
        return (this->size() > 0U);
      }      
      
    };

    // Specialisation for BirthDeath is same as for array:
    template<typename Value>
    class Container<Value, ContainerType::BirthDeath, 1> : public Container<Value, ContainerType::Array, 1> {};
    
    // Not valid but e.g. ContainerBirthDeath = Container<Value, ContainerType::Array, 1> would be:
    // template<typename Value>
    // using Container<Value, ContainerType::BirthDeath, 1> = Container<Value, ContainerType::Array, 1>;
    // Partial specialisation of templates is not allowed:
    // template<typename Value>
    // using typename Container<Value, ContainerType::BirthDeath, 1> = Container<Value, ContainerType::Array, 1>;
        
  } // namespace internal

} // namespace blofeld


// clang complains if we define std::ssize out of line:
namespace std
{
  // Function overload for free-function std::ssize:
  // NOTE: this just avoids static casting via std::size_t - not sure if it actually makes a difference
  template <blofeld::internal::BlofeldContainer C>
  constexpr auto ssize(const C& c)
    -> std::ptrdiff_t // the common type of std::ptrdiff_t and int, which is returned by ssize()
  {
    // Explicit cast from int:
    return static_cast<std::ptrdiff_t>(c.ssize());
  }
} // namespace std

#endif // BLOFELD_CONTAINER_H
