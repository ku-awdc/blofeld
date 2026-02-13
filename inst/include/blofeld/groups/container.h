#ifndef BLOFELD_CONTAINER_H
#define BLOFELD_CONTAINER_H

#include <array>
#include <vector>
#include <stdexcept>

#include "./compartment_types.h"

namespace blofeld
{
  
  // This should not be used externally as inheriting from std::array etc invites misuse
  // NOTE: this may be evil, but we will never hold the std::array as a pointer...
  
  // TODO: implement ssize where needed
  
  // TODO: make this more robust
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
  concept Fixedsize = Container<T> && !Resizeable<T>;
  
  
  namespace internal {
    
    // General container class:
    template<typename Value, ContainerType s_cont_type, int s_n>
    class Container
    {
      Container() = delete; //("Failed to match valid container type");
    };

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
      
      [[nodiscard]] static constexpr auto validate() noexcept
        -> bool
      {
        return true;
      }

      [[nodiscard]] static constexpr auto isActive() noexcept
        -> bool
      {
        return false;
      }
      
    };

    // Specialisation for array is just a std::array
    template<typename Value, int s_n>
    class Container<Value, ContainerType::Array, s_n> : public std::array<Value, s_n>
    {
    public:
      using ReturnType = std::array<Value, s_n>;
      
      Container()
      {
        static_assert(s_n > 0, "Invalid s_n <= 0 for Container<ContainerType::Array>");
        this->fill(static_cast<Value>(0.0));
      }
            
      void zero() noexcept
      {
        this->fill(static_cast<Value>(0.0));
      }
      
      [[nodiscard]] auto validate() const noexcept
        -> bool
      {
        bool valid = true;
        for (auto const& val : (*this)) valid = valid && val >= static_cast<Value>(0.0);
        return valid;
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
      
      [[nodiscard]] static constexpr auto max() noexcept
      {
        return s_max;
      }

      [[nodiscard]] auto isActive() const noexcept
        -> bool
      {
        return (size() > 0U);
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
        throw std::logic_error("Unable to call swap on a Container<ContainerType::InplaceVector> (until I swap my implementation for C++26 inplace_vector)");
      }      
      void back()
      {
        throw std::logic_error("Unable to call swap on a Container<ContainerType::InplaceVector> (until I swap my implementation for C++26 inplace_vector)");
      }
      
    };

    // Specialisation for vector:
    template<typename Value, int s_n>
    class Container<Value, ContainerType::Vector, s_n> : public std::vector<Value>
    {
    public:
      Container()
      {
        this->resize(static_cast<std::size_t>(s_n)); // Unnecessary: static_cast<Value>(0.0));
      }
      
      void zero() noexcept
      {
        for (auto& val : (*this)) val = static_cast<Value>(0.0);
      }
      
      [[nodiscard]] auto validate() const noexcept
        -> bool
      {
        bool valid = true;
        for (auto const& val : (*this)) valid = valid && val >= static_cast<Value>(0.0);
        return valid;
      }
      
      [[nodiscard]] auto isActive() const noexcept
        -> bool
      {
        return (this->size() > 0U);
      }      
      
    };

    // Specialisation for BirthDeath - same as array, except no check for non-negative
    template<typename Value>
    class Container<Value, ContainerType::BirthDeath, 1> : public Container<Value, ContainerType::Array, 1>
    {
    public:
      
      // Trivial - all values are valid (except NaN??)
      [[nodiscard]] static constexpr auto validate() noexcept
        -> bool
      {
        return true;
      }
      
    };
    
  } // namespace internal

} // namespace blofeld

#endif // BLOFELD_CONTAINER_H
