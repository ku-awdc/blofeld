// clang++ -Wall -std=c++17 -o test test.cpp; ./test

#include <string_view>
using namespace std::literals::string_view_literals;

#include "MetaPop.h"

int main()
{
  MetaPop<int, std::string_view, float> test(10, "hello"sv, 42.0);
  test.dostuff();
  
  return 0;
}
