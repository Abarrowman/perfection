#include <iostream>
#include <typeinfo>

constexpr bool is_different_from_others() {
  return true;
}

template<typename L>
constexpr bool is_different_from_others() {
  return true;
}

template<typename L, typename R, typename... O>
constexpr bool is_different_from_others() {
  if constexpr (L::value == R::value) {
    return false;
  } else {
    return is_different_from_others<L, O...>();
  }
}

template<typename L>
constexpr bool are_all_different() {
  return true;
}

template<typename L, typename R, typename... O>
constexpr bool are_all_different() {
  if constexpr (is_different_from_others<L, R, O...>()) {
    return are_all_different<R, O...>();
  } else {
    return false;
  }
}


template<typename T, T t>
struct type_value {
  using type = T;
  static constexpr T value = t;
};

#define CONSTANT(T) type_value<decltype(T), T>

int main() {
  using first = type_value<int, 1>;
  using second = type_value<int, 2>;

  bool constexpr different = are_all_different<CONSTANT(1), CONSTANT(2), CONSTANT(3), CONSTANT(3)>();

  std::cout << "Were all different? " << different << "\n";

  return 0;
}