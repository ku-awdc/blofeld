#ifndef BLOFELD_IDENTICAL_H_
#define BLOFELD_IDENTICAL_H_

#include <stdexcept>
#include <type_traits>

namespace blofeld
{
  // Convenience for making for loops, inspired by GSL:
  using index = std::ptrdiff_t;
  using std::ssize;

  // Similar to the GSL but simplified:
  template <class T, class U>
  constexpr auto symmetric_cast(U u)
    -> T
  {
      const T t = static_cast<T>(u);
      if (static_cast<U>(t) != u) throw std::invalid_argument("Failure of symmetry in symmetric_cast");
      return t;
  }  
  
  // Templated function for testing equality:
  template <typename T, typename U>
  constexpr auto identical(T const& a, U const& b, [[maybe_unused]] double const tol = 1e-6)
    -> bool
  {
    // Prevent implicit casts:
    if constexpr (!std::is_same<T,U>()) return false;
    
    if constexpr (std::is_integral<T>() || std::is_enum<T>()) {
      return a == b;
    } else if constexpr (std::is_floating_point<T>()) {
      return a == 0.0 ? std::abs(b) < tol : std::abs((a-b)/a) < tol; 
    } else {
      static_assert(false, "Unhandled type in identical()");
    }    
  }
  
  
  // Concepts:
  
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
  
  
} // namespace blofeld

#endif // BLOFELD_IDENTICAL_H_
