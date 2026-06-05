#ifndef NODE_CHECK
#define NODE_CHECK

#include "util.h"
#include "Nodes.h"
#include <stdexcept>
#include <string>

namespace my::node::check {

template <typename Node, typename Func = decltype([](Node){})>
requires (node_util::BinaryTraversalConcept<Func, Node>)
void check(Node node, Func func = Func{}) {
  auto checker = [&func](Node node) {
    if (node->left == node->right && node->left != nullptr) {
      throw std::logic_error("left = right");
    }

    if (node->left != nullptr && node->left->parent != node) {
      throw std::logic_error("left subnode parent is incorrect");
    }

    if (node->right != nullptr && node->right->parent != node) {
      throw std::logic_error("right subnode parent is incorrect");
    }

    if (node->parent != nullptr && node->parent->left != node && node->parent->right != node) {
      throw std::logic_error("parent is incorrect");
    }

    func(node);
  };

  traversal::postorder(node, checker);
}

template <typename Node, typename Func = decltype([](Node){})>
requires (node_util::BinomialTraversalConcept<Func, Node>)
void check(Node node, Func func = Func{}) {
  auto checker = [&func](Node node) {
    if (node->next != nullptr && node->next->prev != node) {
      throw std::logic_error("next->prev is incorrect");
    }
    if (node->prev != nullptr && node->prev->next != node) {
      throw std::logic_error("prev->next is incorrect");
    }

    size_t d = 0;

    for (auto p = node->subnodes.front; p != nullptr; p = p->next, ++d) {
      if (p->parent != node) {
        throw std::logic_error("children parent is incorrect");
      }
      if (p->degree != d) {
        throw std::logic_error("children degree is incorrect");
      }
    }

    if (d != node->degree) {
      throw std::logic_error("degree is incorrect");
    }

    func(node);
  };

  traversal::postorder(node, checker);
}

}  // namespace my::nodes::test

#endif