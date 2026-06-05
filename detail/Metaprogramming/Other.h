#ifndef METAPROGRAMMING_OTHER
#define METAPROGRAMMING_OTHER

#include <type_traits>

namespace my::metaprog::utils {

  // Check template instance //

  template<typename, template <typename...> typename>
  struct IsInstanceOf : std::false_type {};
  template<template <typename...> typename T, typename... Args>
  struct IsInstanceOf<T<Args...>, T> : std::true_type {};

}  // namespace my::metaprog


namespace my::metaprog {

  //// Check template instance ////

  template <typename T, template <typename...> typename Instance>
  concept instance_of = utils::IsInstanceOf<T, Instance>::value;

}  // namespace my::metaprog

#endif