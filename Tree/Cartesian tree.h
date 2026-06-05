#ifndef CARTESIAN_TREE
#define CARTESIAN_TREE

#include "../Node manager/Node manager.h"

namespace my {

/**
 * @brief  A data structure that stores pairs (X, Y) in a binary tree in such a way that
 *         it is a binary search tree by X and a binary heap by Y.
 *
 * @tparam  KeyT  Type of key
 * @tparam  PrioT  Type of priority
 * @tparam  CompKey  Type of comparator for keys
 * @tparam  CompPrio  Type of comparator for priorities
 */
template <typename KeyT, typename PrioT, typename CompKey = std::less<KeyT>, typename CompPrio = std::less<PrioT>>
class CartesianTree {

  struct Node {
    KeyT key;
    PrioT prio;

   public:
    template <typename ArgK, typename ArgP>
    Node(ArgK&& key, ArgP&& prio) : key(std::forward<ArgK>(key)), prio(std::forward<ArgP>(prio)) {
    }
  };


 public:
  using key_type = KeyT;
  using prio_type = PrioT;

  using key_compare = CompKey;
  using prio_compare = CompPrio;

  using iterator = BinaryNodeManager<Node>::iterator;
  using const_iterator = BinaryNodeManager<Node>::const_iterator;


 private:
  using node_pointer = typename BinaryNodeManager<Node>::pointer;
  using const_node_pointer = typename BinaryNodeManager<Node>::const_pointer;


 private:
  BinaryNodeManager<Node> node_manager_;


 public:
  CartesianTree() = default;

  /**
   * @brief  Constructor.
   *         Time: Θ(n)
   *
   * @param  data  Range of elements to build the treap on
   */
  template <typename Cont>
  requires (std::ranges::input_range<Cont> && )
  explicit CartesianTree(const Cont& data) {
    node_pointer last = node_manager_.root();

    for (const auto& [key, prio] : data) {
      node_pointer node = node_manager_.new_node(key, prio);
      node_pointer curr = last;

      while (curr != nullptr && curr->prio > node->prio) {
        curr = curr->parent;
      }

      if (curr != nullptr) {
        node->left = curr->right;
        if (curr->right != nullptr) {
          curr->right->parent = node;
        }
        curr->right = node;
        node->parent = curr;

      } else {
        node->left = node_manager_.root();
        if (node_manager_.root() != nullptr) {
          node_manager_.root()->parent = node;
        }
        node_manager_.root() = node;
      }

      last = node;
    }

    while (last != nullptr) {
      last = last->parent;
    }
  }


 private:
  // At //

  node_pointer get(const key_type& key) {
    node_pointer node = node_manager_.root();

    while (node != nullptr) {
      if (CompKey{}(node->key, key)) {
        node = node->right;
      } else if (CompKey{}(key, node->key)) {
        node = node->left;
      } else {
        return node;
      }
    }

    throw std::logic_error("The key was not found");
  }


  // Helpers //

  static void reset_parent(node_pointer node) {
    if (node != nullptr) {
      node->parent = nullptr;
    }
  }

  static void fix_left_parent(node_pointer node) {
    if (node->left != nullptr) {
      node->left->parent = node;
    }
  }
  static void fix_right_parent(node_pointer node) {
    if (node->right != nullptr) {
      node->right->parent = node;
    }
  }


  // Insert function //

  static node_pointer restructure(node_pointer node) {
    if (node == nullptr) {
      return nullptr;
    }

    bool need_left = (node->left != nullptr && CompPrio{}(node->left->prio, node->prio));
    bool need_right = (node->right != nullptr && CompPrio{}(node->right->prio, node->prio));

    if (need_left && need_right) {
      (CompPrio{}(node->left->prio, node->right->prio) ? need_right : need_left) = false;
    }

    if (need_left) {
      node = node->left;
      nodes::rotate::rotate(node);
    }

    if (need_right) {
      node = node->right;
      nodes::rotate::rotate(node);
    }

    return node;
  }


  // Merge & Split functions //

  static node_pointer merge_nodes(node_pointer node_1, node_pointer node_2) {
    if (node_1 == nullptr) {
      reset_parent(node_2);
      return node_2;
    }
    if (node_2 == nullptr) {
      reset_parent(node_1);
      return node_1;
    }

    node_pointer p = nullptr;

    if (CompPrio{}(node_2->prio, node_1->prio)) {
      p = node_2;
      p->left = merge_nodes(node_1, p->left);
      fix_left_parent(p);

    } else {
      p = node_1;
      p->right = merge_nodes(p->right, node_2);
      fix_right_parent(p);
    }

    reset_parent(p);

    return p;
  }

  static std::pair<node_pointer, node_pointer> split_node(node_pointer node, const key_type& key) {
    if (node == nullptr) {
      return {nullptr, nullptr};
    }

    std::pair<node_pointer, node_pointer> twix;

    if (CompKey{}(node->key, key)) {
      auto nodes = split_node(node->right, key);
      node->right = nodes.first;
      fix_right_parent(node);
      reset_parent(node);
      reset_parent(nodes.second);
      twix = std::make_pair(node, nodes.second);

    } else {
      auto nodes = split_node(node->left, key);
      node->left = nodes.second;
      fix_left_parent(node);
      reset_parent(node);
      reset_parent(nodes.first);
      twix = std::make_pair(nodes.first, node);
    }

    return twix;
  }


 public:
  //// ACCESS ////

  iterator operator[](const key_type& key) const {
    return iterator(get(key));
  }


  //// EDIT ////

  template <typename ArgK, typename ArgP>
  iterator insert(ArgK&& key, ArgP&& prio) {
    node_pointer node = node_manager_.new_node(std::forward<ArgK>(key), std::forward<ArgP>(prio));

    node_pointer curr = node_manager_.root();
    node_pointer prev = nullptr;
    bool last_left_turn = false;  // 0 - right, 1 - left

    while (curr != nullptr) {
      prev = curr;

      if (CompKey{}(node->key, curr->key)) {
        curr = curr->left;
        last_left_turn = true;

      } else if (CompKey{}(curr->key, node->key)) {
        curr = curr->right;
        last_left_turn = false;

      } else {
        throw std::logic_error("Such key already exists");
      }
    }

    if (prev != nullptr) {
      (last_left_turn ? prev->left : prev->right) = node;

    } else {
      node_manager_.root() = node;
    }

    node->parent = prev;
    curr = node;

    while (curr != prev) {
      curr = curr->parent;
      prev = curr;
      curr = restructure(curr);
    }

    if (curr == nullptr) {
      node_manager_.root() = node;
    }

    return iterator(node);
  }

  void erase(const key_type& key) {
    erase(iterator(get(key)));
  }

  void erase(iterator it) {
    node_pointer node = it.operator->();

    if (node->parent == nullptr && node->left != node->right) {
      if (node->right == nullptr ||
          (node->left != nullptr && CompPrio{}(node->left->prio, node->right->prio))) {
        node_manager_.root() = node->left;
        nodes::rotate::rotate(node->left);
      } else {
        node_manager_.root() = node->right;
        nodes::rotate::rotate(node->right);
      }
    }

    while (node->left != node->right) {
      if (node->right == nullptr ||
          (node->left != nullptr && CompPrio{}(node->left->prio, node->right->prio))) {
        nodes::rotate::rotate(node->left);
      } else {
        nodes::rotate::rotate(node->right);
      }
    }

    if (node->parent != nullptr) {
      if (node->parent->left == node) {
        node->parent->left = nullptr;
      } else {
        node->parent->right = nullptr;
      }
    }

    node_manager_.del_node(node);
  }

  void merge(CartesianTree& other) {
    node_manager_.merge_alloc(other);
    node_manager_.root() = merge_nodes(node_manager_.root(), other.node_manager_.root());
    other.node_manager_.root() = nullptr;
  }

  // std::pair<CartesianTree, CartesianTree> split(const key_type& key) {
  //   CartesianTree t_1, t_2;

  //   auto [root_1, root_2] = split_node(node_manager_.root(), key);

  //   t_1.node_manager_.root() = root_1;
  //   t_2.node_manager_.root() = root_2;
  //   node_manager_.root() = nullptr;

  //   return std::make_pair(t_1, t_2);
  // }


  //// FIND ////

  iterator find(const key_type& key) const {
    node_pointer node = node_manager_.root();
    node_pointer prev = nullptr;

    while (node != nullptr) {
      if (CompKey{}(node->key, key)) {
        node = node->right;
      } else {
        node = node->left;
      }

      prev = node;
    }

    return iterator(prev);
  }

  iterator upper_bound(const key_type& key) const {
    node_pointer node = node_manager_.root();
    node_pointer prev = nullptr;

    while (node != nullptr) {
      if (CompKey{}(node->key, key)) {
        node = node->right;
      } else {
        node = node->left;
      }

      prev = node;
    }

    return prev != nullptr ? ++iterator(prev) : iterator();
  }

  iterator lower_bound(const key_type& key) const {
    node_pointer node = node_manager_.root();
    node_pointer prev = nullptr;

    while (node != nullptr) {
      if (CompKey{}(node->key, key)) {
        node = node->right;
      } else {
        node = node->left;
      }

      prev = node;
    }

    return prev != nullptr ? --iterator(prev) : iterator();
  }

  bool contains(const key_type& key) const {
    return bool(find(key));
  }


  //// NAVIGATION ////

  iterator begin() {
    const_node_pointer node = node_manager_.root();

    while (node != nullptr && node->left != nullptr) {
      node = node->left;
    }

    return iterator(node);
  }
  iterator end() {
    return iterator();
  }

  const_iterator cbegin() const {
    const_node_pointer node = node_manager_.root();

    while (node != nullptr && node->left != nullptr) {
      node = node->left;
    }

    return const_iterator(node);
  }
  const_iterator cend() const {
    return const_iterator();
  }
};

}  // namespace my

#endif