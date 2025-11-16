#ifndef BLOFELD_BRIDGE_H
#define BLOFELD_BRIDGE_H

#include <sstream>
#include <string_view>


// Formatter for std::array:
template <typename T, size_t S>
class std::formatter<std::array<T,S>> : std::formatter<int>
{
public:
  constexpr auto parse(std::format_parse_context& context)
  {
      return context.begin();
  }

  auto format(std::array<T,S> const& arr, std::format_context& context) const
  {
    auto out = context.out();
    // TODO: show array size not contents if e.g. S>10
    out = std::format_to(out, "arr:{}[", arr.size());
    for (auto vv : arr) {
      out = std::format_to(out, "{}, ", vv);
    }
    return std::format_to(out, ":{}]", std::accumulate(arr.begin(), arr.end(), static_cast<T>(0.0)));
  }
};


namespace blofeld
{

  // Note: this is NOT a pure virtual class because:
  // 1. templated members cannot be virtual
  // 2. we are not using runtime polymorphism anyway
  
  class Bridge
  {
  protected:
    
    template<typename T>
    void stream(std::ostringstream& ss, std::string_view const sep, T const& val)
    {
      //std::string vv = std::format(val);
      ss << val << sep;
    }

    template<typename T, typename... Args>
    void stream(std::ostringstream& ss, std::string_view const sep, T const& t, Args... args)
    {
      stream(ss, sep, t);
      stream(ss, sep, args...);
    }

        
  public:
    void stop(std::string_view const msg);
    void print(std::string_view const msg);
    
    template<size_t s_size>
    std::array<int, s_size+1> rmultinom(int total, std::array<double, s_size> const& probs);
  };

} //blofeld

#endif // BLOFELD_BRIDGE_H
