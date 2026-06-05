#ifndef METAPROGRAMMING_CONTAINERS
#define METAPROGRAMMING_CONTAINERS

#include <cstddef>
#include <type_traits>
#include <tuple>

namespace my::metaprog::utils {

  //// Multicontainers ////

  template <template <typename...> typename Cont, typename T, size_t D>
  struct MultiContainer : std::type_identity<Cont<typename MultiContainer<Cont, T, D - 1>::type>> {};
  template <template <typename...> typename Cont, typename T>
  struct MultiContainer<Cont, T, 0> : std::type_identity<T> {};


  //// Tuple of n equal types ////

  template <typename T, size_t N, typename... Args>
  struct NTuple : std::type_identity<typename NTuple<T, N - 1, T, Args...>::type> {};
  template <typename T, typename... Args>
  struct NTuple<T, 0, Args...> : std::type_identity<std::tuple<Args...>> {};


  //// Tuple ////

  // template <typename TorP, typename... Tail>
  // requires (std::same_as<std::tuple, TorP>)
  // using FlattenType = std::tuple<FlattenType<Tail>...>;

}

namespace my::metaprog {

  //// Multi containers ////

  template <template <typename...> typename Cont, typename T, size_t D>  // WHAT IF CONT HAS INTEGRAL TEMPLATE PARAMETER?
  requires (D > 0)
  using multi = typename utils::MultiContainer<Cont, T, D>::type;


  //// Tuple of n equal types ////

  template <typename T, size_t N>
  using n_tuple = typename utils::NTuple<T, N>::type;

  // n_tuple functions //

  template <typename T, size_t n, std::same_as<T>... Args>
  requires (sizeof...(Args) <= n)
  constexpr n_tuple<T, n> create_tuple(Args&&... args) {
    return std::tuple_cat(std::make_tuple(std::forward<Args>(args)...), n_tuple<T, n - sizeof...(Args)>{});
  }

  template <typename... Args>
  requires (sizeof...(Args) > 0)
  constexpr decltype(auto) pop_front(const std::tuple<Args...>& tuple) {
    return std::apply([](auto, auto... rest) { return std::make_tuple(rest...); }, tuple);
  }

}

#endif