#ifndef RANGE_QUERY_UTILS
#define RANGE_QUERY_UTILS

#include "../detail/Metaprogramming/Containers.h"  // multi, n_tuple, pop_front
#include <ranges>
#include <vector>

namespace my::ranqry_util {

  //// Concepts ////

  // Qurey & update //

  template <typename T>
  concept TypeConcept = std::copy_constructible<T>;

  template <typename Qry, typename T>
  concept QryConcept = requires {
    { std::declval<Qry>()(std::declval<const T>(), std::declval<const T>()) } -> std::convertible_to<T>;
  };

  template <typename Upd, typename T>
  concept UpdConcept = std::same_as<void, Upd> || requires {
    { std::declval<Upd>()(std::declval<T>(), std::declval<T>()) } -> std::convertible_to<T>;
  };

  template <size_t n, typename... Args>
  concept OrdsConcept = (std::is_integral_v<Args>, ...) && sizeof...(Args) == n;


  //// Helper functions ////

  // Segment tree //

  template <typename T, size_t D>
  requires (D > 0)
  using multi_vector = metaprog::multi<std::vector, T, D>;


  //// EXCEPTION ////

  class RangryException : public std::logic_error {
   public:
    RangryException(std::string info) : std::logic_error(info) {
    }
  };

}  // namespace my::ranqry_util

#endif