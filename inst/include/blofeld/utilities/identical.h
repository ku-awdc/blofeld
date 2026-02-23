#ifndef BLOFELD_IDENTICAL_H_
#define BLOFELD_IDENTICAL_H_

#include <type_traits>

namespace blofeld
{
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
      return std::abs((a-b)/a) < tol; 
    } else {
      static_assert(false, "Unhandled type in identical()");
    }    
  }
  
} // namespace blofeld

#endif // BLOFELD_IDENTICAL_H_
