#include <iostream>
#include <typeinfo>
#include <string_view>
#include <cstdint>
#include <vector>
#include <string>
#include <limits>
#include <algorithm>
#include <array>
#include <cassert>
#include <optional>

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
  //std::cout << First::value() << " -> " << hash_value << "\n";

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

template<size_t Count>
void add_strings(std::array<std::string_view, Count>&, varriadic_holder<>, size_t) {}

template<size_t Count, typename F, typename... T>
void add_strings(std::array<std::string_view, Count>& strings, varriadic_holder<F, T...>, size_t index) {
  strings[index] = F::value();
  add_strings(strings, varriadic_holder<T...>{}, index + 1);
}

template<size_t Count, typename... T>
void add_strings(std::array<std::string_view, Count>& strings,  varriadic_holder<T...> vh) {
  add_strings(strings, vh, 0);
}

constexpr uint64_t hash_string(std::string_view str, uint64_t space, uint64_t param) {
  return ((hash_string_view(str) * param) % 15485863) % space;
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
  using STRINGS_T = std::array<std::string_view, TYPES::size()>;

  static constexpr size_t count() {
    return TYPES::size();
  }

  static constexpr uint64_t hash_param() {
    return find_perfect_param(TYPES{}, type_value<uint64_t, 0>{});
  }

  static HASH_TABLE_T const& hash_table() {
    static HASH_TABLE_T table;
    static bool populated = false;
    if (!populated) {
      add_to_table(table, TYPES{}, type_value<uint64_t, hash_param()>{});
    }
    return table;
  }

  static STRINGS_T const& strings() {
    static STRINGS_T strings;
    static bool populated = false;
    if (!populated) {
      add_strings(strings, TYPES{});
    }
    return strings;
  }

  static constexpr uint64_t perfect_hash(std::string_view sv) {
    return hash_string(sv, count(), hash_param());
  }

  static constexpr std::optional<PHONETICS> find(std::string_view sv) {
    auto value = hash_table()[static_cast<size_t>(perfect_hash(sv))];
    if (strings()[static_cast<size_t>(value)] == sv) {
      return value;
    } else {
      return std::nullopt;
    }
  }
};

bool are_all_different(std::vector<uint64_t> const& hashes) {
  for (std::vector<uint64_t>::const_iterator it = hashes.begin(); it != hashes.end(); ++it) {
    if (std::find(it + 1, hashes.end(), *it) != hashes.end()) {
      return false;
    }
  }
  return true;
}

template<size_t Count>
uint64_t find_perfect_hash(std::array<std::string_view, Count> const& strings, std::vector<uint64_t>& hashes) {
  uint64_t max = std::numeric_limits<uint64_t>::max();
  for (uint64_t param = 0; param < max; ++param) {
    /*if (param % 1000 == 0) {
      std::cout << "Trying param=" << param << "\n";
    }*/
    for (size_t j = 0; j < strings.size(); ++j) {
      hashes[j] = hash_string(strings[j], strings.size(), param);
    }
    if (are_all_different(hashes)) {
      return param;
    }
  }
  return max;
}

std::ostream& operator<<(std::ostream& os, phonetics::PHONETICS const& p) {
  size_t value = static_cast<size_t>(p);
  std::string_view str = phonetics::strings()[value];
  os << "PHONETICS(str=\"" << str << "\" value=" << value << ")";
  return os;
}

int main() {
  phonetics::HASH_TABLE_T const& table = phonetics::hash_table();
  phonetics::STRINGS_T const& strings = phonetics::strings();

  std::vector<uint64_t> hashes;
  hashes.resize(strings.size());

  uint64_t param = find_perfect_hash(strings, hashes);
  assert(param == phonetics::hash_param());
  std::cout << "Found the perfect hash function param=" << param << "\n";
  for (size_t j = 0; j < strings.size(); ++j) {
    auto phon = phonetics::find(strings[j]);
    assert(phon.has_value());
    std::cout << "strings[j] = " << strings[j] << "  hashes[j] = " << hashes[j] << "  phonetics.find(strings[j]) = " << phon.value() << "\n";
  }

  assert(phonetics::find("cow") == std::nullopt);

  return 0;
}
