#include <experimental/simd>

#include <iostream>
#include <string_view>

namespace stdx = std::experimental;

void println(std::string_view name, auto const& a)
{
    std::cout << name << ": ";
    for (std::size_t i{}; i != a.size(); ++i)
        std::cout << a[i] << ' ';
    std::cout << '\n';
}


int main()
{
    std::experimental::simd<int> x = 1;
    println("x: ", x);

    std::experimental::simd<int> y = 1;
    y = -y;
    println("y: ", y);
    
    //std::experimental::simd<int> z = x + y;
    //println("z: ", z);
    
    
}

//clang++ -std=c++20 -fexperimental-library simd_test.cpp
