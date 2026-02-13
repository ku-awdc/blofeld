#ifndef BLOFELD_CONTAINER_H
#define BLOFELD_CONTAINER_H

#include <array>
#include <vector>

#include "./ModelCompTypes.h"

namespace blofeld
{
  // This should not be used externally (inheriting from std::array etc invites misuse)
  
  namespace internal {
    
    // General container class:
    template<typename ValueType, CompCont cont_type, size_t s_n>
    class Container
    {
      Container() = delete;
      explicit Container(size_t const n) = delete;
    };
    // TODO: change inheretence to composition???
    // TODO: change size_t s_n to int or std::ptrdiff_t s_n and static_cast on inheretence to array

    // Specialisation for disabled:
    template<typename ValueType>
    class Container<ValueType, CompCont::disabled, 0>
    {      
    public:
      using ReturnType = void;
      
      Container() = default;
      
      //void fill([[maybe_unused]] ValueType const fill_value) const
      void fill(ValueType const) const
      {
      }      
      
    };

    // Specialisation for array is just a std::array
    // NOTE: this may be evil, but we will never hold the std::array as a pointer...
    template<typename ValueType, size_t n>
    class Container<ValueType, CompCont::array, n> : public std::array<ValueType, n> {
    public:
      using ReturnType = std::array<ValueType, n>;
      
      Container()
      {
        this->fill(static_cast<ValueType>(0.0));
      }
      
    };

    // Specialisation for inplace_vector:
    template<typename ValueType, size_t n>
    class Container<ValueType, CompCont::inplace_vector, n> : public std::array<ValueType, n>
    {
    public:
      Container()
      {
        static_assert(false, "CompCont::inplace_vector is not yet implemented");
      }
      
    };

    // Specialisation for vector:
    template<typename ValueType, size_t s_n>
    class Container<ValueType, CompCont::vector, s_n> : public std::vector<ValueType>
    {
      size_t m_n = 0;
      
    public:
      Container()
      {
        resize(s_n);
      }
      
      void resize(size_t const n)
      {
        this->resize(n, static_cast<ValueType>(0.0));
        m_n = n;
      }
    };

    // Specialisation for birth_death - same as array, except no check for non-negative
    template<typename ValueType>
    class Container<ValueType, CompCont::birth_death, 1> : public Container<ValueType, CompCont::array, 1>
    {
    private:
      
    public:
      static constexpr auto validate() noexcept
        -> bool
      {
        return true;
      }
      
    };
    
  } // namespace internal

} // namespace blofeld

#endif // BLOFELD_CONTAINER_H
