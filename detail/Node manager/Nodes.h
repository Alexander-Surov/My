// Func = Visitor

#ifndef NODES
#define NODES

#include "util.h"
#include <cstddef>
#include <type_traits>
#include <utility>
#include <iterator>


namespace my::node {

template <typename Node>
class BinaryNode final : public Node {
 public:
  using pointer = BinaryNode*;
  using const_pointer = const BinaryNode*;


 public:
  pointer left = nullptr;
  pointer right = nullptr;
  pointer parent = nullptr;


 public:
  using Node::Node;

  BinaryNode(const Node& other) : Node(other) {
  }
  BinaryNode(Node&& other) : Node(std::move(other)) {
  }
};


template <typename Node>
class BinomialNode final : public Node {
 public:
  using pointer = BinomialNode*;
  using const_pointer = const BinomialNode*;


 public:
  struct List {
    pointer front = nullptr;
    pointer back = nullptr;

   public:
    void push_front(pointer node) {
      node->next = front;
      node->prev = nullptr;

      if (front == nullptr) {
        front = back = node;
      } else {
        front->prev = node;
        front = node;
      }
    }
    void push_back(pointer node) {
      node->next = nullptr;
      node->prev = back;

      if (back == nullptr) {
        front = back = node;
      } else {
        back->next = node;
        back = node;
      }
    }

    void insert(pointer at, pointer node) {
      node->next = at;
      node->prev = at->prev;
      (at->prev != nullptr ? at->prev->next : front) = node;
      at->prev = node;
    }
    void erase(pointer node) {
      (node->prev != nullptr ? node->prev->next : front) = node->next;
      (node->next != nullptr ? node->next->prev : back) = node->prev;
      node->prev = node->next = nullptr;
    }

    bool empty(pointer parent) const {
      return front == nullptr;
    }
  };


 public:
  size_t degree = 0;
  List subnodes;
  pointer parent = nullptr;
  pointer next = nullptr;
  pointer prev = nullptr;


 public:
  using Node::Node;

  BinomialNode(const Node& other) : Node(other) {
  }
  BinomialNode(Node&& other) : Node(std::move(other)) {
  }
};

}  // namespace my::nodes


namespace my::node::iterator {

template <typename Node, bool is_const>
class BinaryIterator : std::bidirectional_iterator_tag {
 public:
  using value_type = Node;

  using reference = std::conditional_t<is_const, const Node&, Node&>;
  using const_reference = const Node&;

  using pointer = std::conditional_t<is_const, const Node*, Node*>;
  using const_pointer = const Node*;


 private:
  using node_pointer = std::conditional_t<is_const,
                                          typename BinaryNode<Node>::const_pointer,
                                          typename BinaryNode<Node>::pointer>;
  using const_node_pointer = typename BinaryNode<Node>::const_pointer;


 private:
  node_pointer node_ = nullptr;


 public:
  BinaryIterator() = default;

  explicit BinaryIterator(node_pointer p) : node_(p) {
  }

  BinaryIterator(const BinaryIterator<Node, false>& other) : node_(other.node_) {
  }


 public:
  // Access operators //

  reference operator*() {
    return *node_;
  }
  const_reference operator*() const {
    return *node_;
  }

  pointer operator->() {
    return node_;  // use std::static_pointer_cast for std::shared_ptr
  }
  const_pointer operator->() const {
    return node_;
  }


  // In/de-crement operators //

  BinaryIterator& operator++() {
    if (node_->right != nullptr) {
      node_ = node_->right;
      while (node_->left != nullptr) {
        node_ = node_->left;
      }

    } else {
      while (node_->parent != nullptr && node_->parent->right == node_) {
        node_ = node_->parent;
      }
      node_ = node_->parent;
    }

    return *this;
  }
  BinaryIterator operator++(int) {
    BinaryIterator it = *this;
    (void) operator++();
    return it;
  }

  BinaryIterator& operator--() {
    if (node_->left != nullptr) {
      node_ = node_->left;
      while (node_->right != nullptr) {
        node_ = node_->right;
      }

    } else {
      while (node_->parent != nullptr && node_->parent->left == node_) {
        node_ = node_->parent;
      }
      node_ = node_->parent;
    }

    return *this;
  }
  BinaryIterator operator--(int) {
    BinaryIterator it = *this;
    (void) operator--();
    return it;
  }


  // Equality operators //

  bool operator==(const BinaryIterator& it) const {
    return node_ == it.node_;
  }
  bool operator!=(const BinaryIterator& it) const {
    return node_ != it.node_;
  }
};


template <typename Node, bool is_const>
class BinomialIterator : std::bidirectional_iterator_tag {
 public:
  using value_type = Node;

  using reference = std::conditional_t<is_const, const Node&, Node&>;
  using const_reference = const Node&;

  using pointer = std::conditional_t<is_const, const Node*, Node*>;
  using const_pointer = const Node*;


 private:
  using node_pointer = std::conditional_t<is_const,
                                          typename BinomialNode<Node>::const_pointer,
                                          typename BinomialNode<Node>::pointer>;
  using const_node_pointer = typename BinomialNode<Node>::const_pointer;


 private:
  node_pointer node_ = nullptr;


 public:
  BinomialIterator() = default;

  explicit BinomialIterator(node_pointer p) : node_(p) {
  }

  BinomialIterator(const BinomialIterator<Node, false>& other) : node_(other.node_) {
  }


 public:
  // Access operators //

  reference operator*() {
    return *node_;
  }
  const_reference operator*() const {
    return *node_;
  }

  pointer operator->() {
    return node_;
  }
  const_pointer operator->() const {
    return node_;
  }


  // In/de-crement operators //

  BinomialIterator& operator++() {
    if (node_->front_subnode != nullptr) {
      node_ = node_->front_subnode;

    } else if (node_->next != nullptr) {
      node_ = node_->next;

    } else {
      while (node_->parent != nullptr && node_->parent->next == nullptr) {
        node_ = node_->parent;
      }

      if (node_->parent != nullptr) {
        node_ = node_->parent->next;
      } else {
        node_ = nullptr;
      }
    }

    return *this;
  }
  BinomialIterator operator++(int) {
    BinaryIterator it = *this;
    (void) operator++();
    return it;
  }

  BinomialIterator& operator--() {
    if (node_->back_subnode != nullptr) {
      node_ = node_->back_subnode;

    } else if (node_->prev == nullptr) {
      node_ = node_->parent;

    } else {
      node_ = node_->prev;

      if (node_ != nullptr) {
        while (node_->back_subnode != nullptr) {
          node_ = node_->back_subnode;
        }
      }
    }

    return *this;
  }
  BinomialIterator operator--(int) {
    BinomialIterator it = *this;
    (void) operator--();
    return it;
  }


  // Equality operators //

  bool operator==(const BinomialIterator& it) const {
    return node_ == it.node_;
  }
  bool operator!=(const BinomialIterator& it) const {
    return node_ != it.node_;
  }
};

}  // namespace my::nodes::iterator


namespace my::node::traversal {

//// GENERAL TEMPLATE ////

template <typename Func, typename Node>
void inorder(Node, Func) {
  throw;
}

template <typename Func, typename Node>
void preorder(Node, Func) {
  throw;
}

template <typename Func, typename Node>
void postorder(Node, Func) {
  throw;
}


//// SPECIALIZATIONS ////

// Binary node //

template <typename Func, typename Node>
requires (node_util::BinaryTraversalConcept<Func, Node>)
void inorder(Node node, Func func) {
  if (node == nullptr) {
    return;
  }

  inorder(node->left, func);
  func(node);
  inorder(node->right, func);
}

template <typename Func, typename Node>
requires (node_util::BinaryTraversalConcept<Func, Node>)
void preorder(Node node, Func func) {
  if (node == nullptr) {
    return;
  }

  func(node);
  preorder(node->left, func);
  preorder(node->right, func);
}

template <typename Func, typename Node>
requires (node_util::BinaryTraversalConcept<Func, Node>)
void postorder(Node node, Func func) {
  if (node == nullptr) {
    return;
  }

  postorder(node->left, func);
  postorder(node->right, func);
  func(node);
}


// Binomial node //

template <typename Func, typename Node>
requires (node_util::BinomialTraversalConcept<Func, Node>)
void preorder(Node node, Func func) {
  if (node == nullptr) {
    return;
  }

  func(node);
  
  for (auto p = node->subnodes.front; p != nullptr; p = p->next) {
    preorder(p, func);
  }
}

template <typename Func, typename Node>
requires (node_util::BinomialTraversalConcept<Func, Node>)
void postorder(Node node, Func func) {
  if (node == nullptr) {
    return;
  }

  for (auto p = node->subnodes.front; p != nullptr; p = p->next) {
    postorder(p, func);
  }

  func(node);
}

}  // namespace my::nodes::traversal


namespace my::node::rotate {

template <typename Node>
requires (node_util::BinaryRecogConcept<Node>)
void rotate(Node node) {
  if (node == nullptr || node->parent == nullptr) {
    return;
  }

  auto parent = node->parent;
  auto grand = parent->parent;

  if (grand != nullptr) {
    (grand->left == parent ? grand->left : grand->right) = node;
  }

  if (parent->left == node) {
    // zig ↷
    parent->left = node->right;
    if (node->right != nullptr) {
      node->right->parent = parent;
    }
    node->right = parent;

  } else {
    // zag ↶
    parent->right = node->left;
    if (node->left != nullptr) {
      node->left->parent = parent;
    }
    node->left = parent;
  }

  parent->parent = node;
  node->parent = grand;
}

}  // namespace my::nodes::rotate

#endif