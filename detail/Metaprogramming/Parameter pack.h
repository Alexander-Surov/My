#ifndef METAPROGRAMMING_PARAMETER_PACK
#define METAPROGRAMMING_PARAMETER_PACK

#include <cstddef>
#include <type_traits>

namespace my::metaprog::util {

  // Type by index //

  template <size_t idx, typename A, typename... Args>
  struct TypeByIndex : std::type_identity<typename TypeByIndex<idx - 1, Args...>::type> {};
  template <typename A, typename... Args>
  struct TypeByIndex<0, A, Args...> : std::type_identity<A> {};

  // Index by type //

  template <typename T, typename A, typename... Args>
  struct IndexByType : std::integral_constant<size_t, std::is_same_v<T, A> ? 0 : IndexByType<T, Args...>::value + 1> {};
  template <typename T, typename A>
  struct IndexByType<T, A> : std::integral_constant<size_t, 0> {};

  // Count type occurrences //

  template <typename T, typename A, typename... Args>
  struct CountType : std::integral_constant<size_t, CountType<T, Args...>::value + std::is_same_v<T, A>> {};
  template <typename T, typename A>
  struct CountType<T, A> : std::integral_constant<size_t, std::is_same_v<T, A>> {};

}

namespace my::metaprog {

  //// Type by index ////

  template <size_t idx, typename... Args>
  requires(idx + 1 <= sizeof...(Args))
  using type_at = typename util::TypeByIndex<idx, Args...>::type;

  //// Index by type ////

  template <typename T, typename... Args>
  requires(sizeof...(Args) > 0 && (std::is_same_v<T, Args> | ...))
  constexpr size_t index_of = util::IndexByType<T, Args...>::value;

  //// Contains a type ////

  template <typename T, typename... Args>
  requires(sizeof...(Args) > 0)
  constexpr bool contains_type = (std::is_same_v<T, Args> | ...);

  //// Count type occurrences ////

  template <typename T, typename... Args>
  requires(sizeof...(Args) > 0)
  constexpr size_t count_type = util::CountType<T, Args...>::value;

  //// Unique type ////

  template <typename T, typename... Args>
  requires(sizeof...(Args) > 0)
  constexpr bool is_unique = (count_type<T, Args...> == 1);

}

#endif