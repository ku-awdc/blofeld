#ifndef BLOFELD_CONTAINER_FORMATTER_H
#define BLOFELD_CONTAINER_FORMATTER_H

#include <format>
#include <array>

#include "../groups/container.h"
// #include "../groups/compartment.h"

namespace blofeld
{
  namespace internal
  {
    // Generic formatter for container types:
    template<class C>
    class ContainerFormatter : public std::formatter<int>
    {
    public:
      constexpr auto parse(std::format_parse_context& context)
      {
          return context.begin();
      }

      auto format(C const& ctr, std::format_context& context) const
      {
        auto out = context.out();
        out = std::format_to(out, "[");        
        
        if constexpr (BlofeldContainer<C>) {
          out = std::format_to(out, "BF ");
        }
        if constexpr (Resizeable<C>) {
          if constexpr (Maxsize<C>) {
            out = std::format_to(out, "Semi-resizeable container: ");
          } else {
            out = std::format_to(out, "Resizeable container: ");            
          }
        } else {
          out = std::format_to(out, "Fixed-size container: ");
        }
        
        // TODO: show container size not contents if e.g. C.size()>10
        //out = std::format_to(out, "[s:{},", ctr.size());
        //out = std::format_to(out, "[");
        for (auto vv : ctr) {
          out = std::format_to(out, "{},", vv);
        }
        // out = std::format_to(out, "t:{}]", std::accumulate(ctr.begin(), ctr.end(), static_cast<C::value_type>(0)));
        std::format_to(out, "s:{}]", ctr.size());
        return out;
      }
    };

  }
}

// Formatter for std::vector:
template<typename T>
class std::formatter<std::vector<T>> : public blofeld::internal::ContainerFormatter<std::vector<T>> {};

// Formatter for std::array:
template<typename T, size_t S>
class std::formatter<std::array<T,S>> : public blofeld::internal::ContainerFormatter<std::array<T,S>> {};

// Formatter for blofeld::Container:
template<typename Value, blofeld::ContainerType s_cont_type, int s_n>
class std::formatter<blofeld::internal::Container<Value, s_cont_type, s_n>> : public blofeld::internal::ContainerFormatter<blofeld::internal::Container<Value, s_cont_type, s_n>> {};

/*
// Formatter for blofeld::Compartment:
template<auto s_cts, blofeld::ModelType s_mtype, blofeld::CompType s_ctype>
class std::formatter<blofeld::Compartment<s_cts, s_mtype, s_ctype>> : public blofeld::ContainerFormatter<blofeld::Compartment<s_cts, s_mtype, s_ctype>> {};
*/

#endif // BLOFELD_CONTAINER_FORMATTER_H
