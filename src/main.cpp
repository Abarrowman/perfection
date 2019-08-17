#include <iostream>
#include <typeinfo>
#include <string_view>
#include <cstdint>

#include <vector>
#include <string>
#include <limits>

#include <algorithm>
#include <array>

template<typename L>
constexpr bool is_different_from_others() {
  return true;
}

template<typename L, typename R, typename... O>
constexpr bool is_different_from_others() {
  if constexpr (L::value() == R::value()) {
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
  static constexpr T value() {
    return t;
  }
};

#define CONSTANT(T) type_value<decltype(T), T>

#define DECLARE_HOLDER(T) struct T { \
  static constexpr auto value() { \
    return std::string_view(#T); \
  } \
};

// From MSVC C++ standard library
constexpr uint32_t hash_string_view(std::string_view sv) {
  uint32_t hash = 2166136261U;
  uint32_t prime = 16777619U;
  for (uint32_t i = 0; i < sv.size(); ++i) {
    hash ^= sv[i];
    hash *= prime;
  }
  return hash;
}

template<typename S, uint64_t space, uint64_t param>
constexpr auto hash_string_constant() {
  constexpr uint64_t hash_value = ((hash_string_view(S::value()) * param) % 15485863) % space;
  return type_value<uint64_t, hash_value>{};
}

template<typename... A>
struct varriadic_holder{
  static constexpr size_t size() {
    return sizeof...(A);
  }
};

//#define DECLARE_PERFECT_ENUM(NAME, ...) enum class NAME { __VA_ARGS__ }; using NAME ## _LIST = varriadic_holder<__VA_ARGS__>;

#define DECLARE_ENUM_HOLDER(E, T, S) struct T { \
  static constexpr auto value() { \
    return std::string_view(S); \
  } \
  static constexpr auto enum_value() { \
    return E::T; \
  } \
};


template<typename Enum, uint64_t Count, uint64_t Param>
void inner_add_to_table(std::array<Enum, Count>& table) {}

template<typename Enum, uint64_t Count, uint64_t Param, typename First, typename... T>
void inner_add_to_table(std::array<Enum, Count>& table) {

  auto hash = hash_string_constant<First, Count, Param>();
  constexpr uint64_t hash_value = decltype(hash)::value();
  std::cout << First::value() << " -> " << hash_value << "\n";

  table[hash_value] = First::enum_value();

  inner_add_to_table<Enum, Count, Param, T...>(table);
}

template<typename Enum, uint64_t Count, uint64_t Param, typename First, typename... T>
void add_to_table(std::array<Enum, Count>& table, varriadic_holder<First, T...>, type_value<uint64_t, Param>) {
  inner_add_to_table<Enum, Count, Param, First, T...>(table);
}




template<typename First, typename... T>
constexpr auto merge(First first, varriadic_holder<T...> others) {
  return varriadic_holder<First, T...>{};
}


template<uint64_t Param, uint64_t Count>
constexpr auto map_hash_string_constant() {
  return varriadic_holder<>{};
}

template<uint64_t Param, uint64_t Count, typename First, typename... L>
constexpr auto map_hash_string_constant() {

  auto mine = hash_string_constant<First, Count, Param>();
  auto others = map_hash_string_constant<Param, Count, L...>();

  return merge(mine, others);
}

template<typename... T>
constexpr bool unwrap_are_all_different(varriadic_holder<T...> vh) {
  return are_all_different<T...>();
}

template<uint64_t Param, uint64_t Count, typename... L>
constexpr bool perfectly_hashes() {
  auto hashed = map_hash_string_constant<Param, Count, L...>();
  return unwrap_are_all_different(hashed);
}

template<uint64_t Start, uint64_t Count, typename... L>
constexpr uint64_t inner_find_perfect_param() {
  if constexpr (Start == 1000) { // hard coded limit
    return Start;
  } else if constexpr (perfectly_hashes<Start, Count, L...>()) {
    return Start;
  } else {
    return inner_find_perfect_param<Start + 1, Count, L...>();
  }
}


template<uint64_t Start, typename... L>
static constexpr uint64_t find_perfect_param(varriadic_holder<L...> vh, type_value<uint64_t, Start>) {
  return inner_find_perfect_param<Start, decltype(vh)::size(), L...>();
}


struct phonetics {
  enum class PHONETICS { ALFA, BRAVO, CHARLIE, DELTA, ECHO, FOXTROT, GOLF, HOTEL };
  DECLARE_ENUM_HOLDER(PHONETICS, ALFA, "alpha")
  DECLARE_ENUM_HOLDER(PHONETICS, BRAVO, "bravo")
  DECLARE_ENUM_HOLDER(PHONETICS, CHARLIE, "charlie")
  DECLARE_ENUM_HOLDER(PHONETICS, DELTA, "delta")
  DECLARE_ENUM_HOLDER(PHONETICS, ECHO, "echo")
  DECLARE_ENUM_HOLDER(PHONETICS, FOXTROT, "fox-trot")
  DECLARE_ENUM_HOLDER(PHONETICS, GOLF, "golf")
  DECLARE_ENUM_HOLDER(PHONETICS, HOTEL, "hotel")
  using TYPES = varriadic_holder<ALFA, BRAVO, CHARLIE, DELTA, ECHO, FOXTROT, GOLF, HOTEL>;
  using HASH_TABLE_T = std::array<PHONETICS, TYPES::size()>;


  static constexpr size_t count() {
    return TYPES::size();
  }

  static constexpr uint64_t hash_param() {
    return find_perfect_param(TYPES{}, type_value<uint64_t, 0>{});
  }

  static HASH_TABLE_T const& get_hash_table() {
    static HASH_TABLE_T table;
    static bool populated = false;
    if (!populated) {
      add_to_table(table, TYPES{}, type_value<uint64_t, hash_param()>{});
    }
    return table;
  }

  /*std::ostream& operator<<(std::ostream& os, PHONETICS const& p) {
    uint64_t idx = static_cast<uint64_t>(p);
    std::string_view name;
    return os;
  }*/
};

uint64_t hash_string(std::string_view str, uint64_t space, uint64_t param) {
  return ((hash_string_view(str) * param) % 15485863) % space;
}

bool are_all_different(std::vector<uint64_t> const& hashes) {
  for (std::vector<uint64_t>::const_iterator it = hashes.begin(); it != hashes.end(); ++it) {
    if (std::find(it + 1, hashes.end(), *it) != hashes.end()) {
      return false;
    }
  }
  return true;
}

uint64_t find_perfect_hash(std::vector<std::string_view> const& strings, std::vector<uint64_t>& hashes) {
  uint64_t max = std::numeric_limits<uint64_t>::max();
  for (uint64_t param = 0; param < max; ++param) {
    if (param % 1000 == 0) {
      std::cout << "Trying param=" << param << "\n";
    }
    for (size_t j = 0; j < strings.size(); ++j) {
      hashes[j] = hash_string(strings[j], strings.size(), param);
    }
    if (are_all_different(hashes)) {
      return param;
    }
  }
  return max;
}

void add_strings(std::vector<std::string_view>&, varriadic_holder<>) {}

template<typename F, typename... T>
void add_strings(std::vector<std::string_view>& vec, varriadic_holder<F, T...>) {
  vec.emplace_back(F::value());
  add_strings(vec, varriadic_holder<T...>{});
}

template<typename... T>
std::vector<std::string_view> strings_from_list(varriadic_holder<T...> vh) {
  std::vector<std::string_view> vec;
  add_strings(vec, vh);
  return vec;
}

int main() {
  phonetics::HASH_TABLE_T const& table = phonetics::get_hash_table();


  std::vector<std::string_view> strings = strings_from_list(phonetics::TYPES{});

  std::vector<uint64_t> hashes;
  hashes.resize(strings.size());

  uint64_t param = find_perfect_hash(strings, hashes);

  std::cout << "Found the perfect hash function param=" << param << "\n";
  for (size_t j = 0; j < strings.size(); ++j) {
    std::cout << "strings[j] = " << strings[j] << "  hashes[j] = " << hashes[j] << "\n";
//    std::cout << "strings[j] = " << strings[j] << "  hashes[j] = " << hashes[j] << "  table[hashes[j]] = " << table[hashes[j]] << "\n";
  }

  return 0;
}
