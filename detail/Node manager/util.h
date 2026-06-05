#ifndef NODE_UTIL
#define NODE_UTIL

#include <concepts>

namespace my::node_util {

//// CONCEPTS ////

// Node //

template <typename Nd>
concept NodeConcept = requires {
  requires std::is_class_v<Nd>;
  requires !std::is_final_v<Nd>;
  requires std::is_copy_constructible_v<Nd>;
};


// CRTP specifications //

template <typename Nd>
concept BinaryNodeConcept = NodeConcept<Nd> && !requires {
  std::declval<Nd>().left;
  std::declval<Nd>().right;
  std::declval<Nd>().parent;
};

template <typename Nd>
concept BinomialNodeConcept = NodeConcept<Nd> && !requires {
  std::declval<Nd>().degree;
  std::declval<Nd>().first_subnode;
  std::declval<Nd>().last_subnode;
  std::declval<Nd>().parent;
  std::declval<Nd>().next;
  std::declval<Nd>().prev;
};


// Recognition //

template <typename Node>
concept BinaryRecogConcept = requires {
  { std::declval<Node>() == nullptr };
  { std::declval<Node>() != nullptr };

  { std::declval<Node>()->left } -> std::convertible_to<Node>;
  { std::declval<Node>()->right } -> std::convertible_to<Node>;
  { std::declval<Node>()->parent } -> std::convertible_to<Node>;
};

template <typename Node>
concept BinomialRecogConcept = requires {
  { std::declval<Node>() == nullptr };
  { std::declval<Node>() != nullptr };

  { std::declval<Node>()->degree } -> std::convertible_to<std::size_t>;
  { std::declval<Node>()->subnodes.front } -> std::convertible_to<Node>;
  { std::declval<Node>()->subnodes.back } -> std::convertible_to<Node>;
  { std::declval<Node>()->parent } -> std::convertible_to<Node>;
  { std::declval<Node>()->next } -> std::convertible_to<Node>;
  { std::declval<Node>()->prev } -> std::convertible_to<Node>;
};


// Namespaced functions //

template <typename Func, typename Node>
concept BinaryTraversalConcept = BinaryRecogConcept<Node> && requires {
  requires std::invocable<Func, Node>;
};

template <typename Func, typename Node>
concept BinomialTraversalConcept = BinomialRecogConcept<Node> && requires {
  requires std::invocable<Func, Node>;
};

}  // namespace node_util

#endif