cat > simd_test.cpp << EOF
#include <experimental/simd>
template <typename T, class Abi>
struct TypeTraits<std::experimental::simd_mask<T, Abi>> {
  using IndexType = typename std::experimental::simd_mask<T, Abi>::simd_type;
  using ScalarType = typename std::experimental::simd_mask<T, Abi>::value_type;
  static constexpr size_t Size = std::experimental::simd<T, Abi>::size();
};
int main() {
  return 0;
}
EOF

clang++ -std=c++20 simd_test.cpp
clang++ -std=c++20 -fexperimental-library simd_test.cpp
