#ifndef IMPLICIT_TREAP
#define IMPLICIT_TREAP

#include "util.h"
#include "../Node manager/Node manager.h"
#include <optional>


namespace my {

/**
 * @brief   A data structure that stores array as a binary tree.
 *
 * @note   These properties should be satisfied:
 *         (a ⊕ b) ⊙ c = (a ⊙ c) ⊕ (b ⊙ c)
 *         (a ⊙ b) ⊙ c = a ⊙ (b ⊙ c)
 *
 * @tparam  T  Type of data
 * @tparam  Qry  Type of query operation (⊕)
 * @tparam  Upd  Type of update operation (⊙)
 */
template <typename T, typename Qry, typename Upd = void>
class ImplicitTreap {

  struct Node {
    T value;
    T result;
    std::optional<T> promise;
    size_t size;
    size_t rnd_priority;

   public:
    template <typename Arg>
    Node(Arg&& value) : value(value), result(value), size(1), rnd_priority(std::rand()) {
    }
  };


 public:
  using value_type = T;

  using qry_type = Qry;
  using upd_type = Upd;

  using iterator = BinaryNodeManager<Node>::const_iterator;
  using const_iterator = BinaryNodeManager<Node>::const_iterator;


 private:
  using node_pointer = typename BinaryNodeManager<Node>::pointer;
  using const_node_pointer = typename BinaryNodeManager<Node>::const_pointer;


 private:
  static constexpr bool is_updatable = !std::is_same_v<void, Upd>;


 private:
  BinaryNodeManager<Node> node_manager_;
  size_t sz_ = 0;


 public:
  ImplicitTreap() = default;

  /**
   * @brief  Constructor.
   *         Time: Θ(n)
   *
   * @param  data  Range of elements to build the treap on
   */
  template <typename Cont>
  requires (std::ranges::input_range<Cont>)
  explicit ImplicitTreap(const Cont& data) {
    node_pointer last = node_manager_.root();

    for (const auto& value : data) {
      ++sz_;
      node_pointer node = node_manager_.new_node(value);
      node_pointer curr = last;

      while (curr != nullptr && curr->rnd_priority > node->rnd_priority) {
        fix_node(curr);
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
      fix_node(last);
    }

    while (last != nullptr) {
      fix_node(last);
      last = last->parent;
    }
  }

 private:
  // At //

  static size_t node_index(node_pointer node) {
    return node->left != nullptr ? node->left->size : 0;
  }

  node_pointer get(size_t i) {
    node_pointer node = node_manager_.root();

    while (push_node(node), node != nullptr && node_index(node) != i) {
      if (node_index(node) < i) {
        i -= node_index(node) + 1;
        node = node->right;
      } else {
        node = node->left;
      }
    }

    return node;
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


  // Balancers //

  static void fix_node(node_pointer node) {
    if (node == nullptr) {
      return;
    }

    // promises are not counted: push beforehand
    switch ((node->left != nullptr) << 1 | (node->right != nullptr)) {
      case 0b00:
        node->result = node->value;
        node->size = 1;
        break;
      case 0b01:
        node->result = Qry{}(node->value, node->right->result);
        node->size = 1 + node->right->size;
        break;
      case 0b10:
        node->result = Qry{}(node->left->result, node->value);
        node->size = 1 + node->left->size;
        break;
      default:
        node->result = Qry{}(Qry{}(node->left->result, node->value), node->right->result);
        node->size = 1 + node->left->size + node->right->size;
    }
  }

  static void push_node(node_pointer node) {
    if constexpr (is_updatable) {
      if (node == nullptr || !node->promise.has_value()) {
        return;
      }

      node->value = Upd{}(node->value, node->promise);
      node->result = Upd{}(node->result, node->promise);

      if (node->left != nullptr) {
        node->left->promise = Upd{}(node->left->promise, node->promise);
      }
      if (node->right != nullptr) {
        node->right->promise = Upd{}(node->right->promise, node->promise);
      }

      node->promise.reset();
    }
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

    if (node_2->rnd_priority < node_1->rnd_priority) {
      p = node_2;
      push_node(p);
      p->left = merge_nodes(node_1, p->left);
      fix_left_parent(p);

    } else {
      p = node_1;
      push_node(p);
      p->right = merge_nodes(p->right, node_2);
      fix_right_parent(p);
    }

    fix_node(p);
    reset_parent(p);

    return p;
  }

  static std::pair<node_pointer, node_pointer> split_node(node_pointer node, size_t i) {
    if (node == nullptr) {
      return {nullptr, nullptr};
    }

    std::pair<node_pointer, node_pointer> twix;

    push_node(node);

    if (node_index(node) < i) {
      auto nodes = split_node(node->right, i - node_index(node) - 1);
      node->right = nodes.first;
      fix_right_parent(node);
      fix_node(node);
      fix_node(nodes.second);
      reset_parent(node);
      reset_parent(nodes.second);
      twix = std::make_pair(node, nodes.second);

    } else {
      auto nodes = split_node(node->left, i);
      node->left = nodes.second;
      fix_left_parent(node);
      fix_node(node);
      fix_node(nodes.first);
      reset_parent(node);
      reset_parent(nodes.first);
      twix = std::make_pair(nodes.first, node);
    }

    return twix;
  }


 public:
  //// ACCESSORS ////

  /**
   * @brief  Subscript access to the data contained in the structure.
   *         Time: O(log(n))
   *
   * @param  i  Index
   */
  iterator operator[](size_t i) const {
    const_node_pointer node = node_manager_.root();

    while (push_node(node), node_index(node) != i) {
      if (node_index(node) < i) {
        i -= node_index(node) + 1;
        node = node->right;
      } else {
        node = node->left;
      }
    }

    return iterator(node);
  }

  /**
   * @brief  Returns the number of elements stored in the structure.
   *         Time: Θ(1)
   */
  size_t size() const {
    return sz_;
  }


  //// EDITORS ////

  /**
   * @brief  This function will insert a copy of the given value before the specified location.
   *         Time: O(log(n))
   *
   * @param  i      Index
   * @param  value  Data to be inserted
   */
  template <typename Arg>
  iterator insert(size_t i, Arg&& value) {
    node_pointer node = node_manager_.new_node(std::forward<Arg>(value));

    auto [left, right] = split_node(node_manager_.root(), i);
    node_manager_.root() = merge_nodes(merge_nodes(left, node), right);

    ++sz_;

    return iterator(node);
  }

  /**
   * @brief  Removes element at the given position.
   *         Time: O(log(n))
   *
   * @param  i  Index
   */
  void erase(size_t i) {
    auto [left, node_right] = split_node(node_manager_.root(), i);
    auto [node, right] = split_node(node_right, i - left->size + 1);

    node->left = nullptr;
    node->right = nullptr;
    node_manager_.del_node(node);

    node_manager_.root() = merge_nodes(left, right);

    --sz_;
  }

  /**
   * @brief  Appends data from the other structure.
   *         Time: O(log(n))
   *
   * @note   The other structure remains valid but empty.
   *
   * @param  other  Other structure
   */
  // void merge(ImplicitTreap& other) {
  //   node_manager_.root() = merge_nodes(node_manager_.root(), other.node_manager_.root());

  //   other.node_manager_.root() = nullptr;

  //   sz_ += other.sz_;
  //   other.sz_ = 0;
  // }

  /**
   * @brief  Splits array at the index.
   *         Time: O(log(n))
   * 
   * @note   The current structure remains valid but empty.
   *
   * @param  i  Index
   */
  // std::pair<ImplicitTreap, ImplicitTreap> split(size_t i) {
  //   ImplicitTreap t_1, t_2;

  //   auto [root_1, root_2] = split_node(node_manager_.root(), i);

  //   t_1.node_manager_.root() = root_1;
  //   t_2.node_manager_.root() = root_2;
  //   node_manager_.root() = nullptr;

  //   t_1.sz_ = i;
  //   t_2.sz_ = sz_ - i;
  //   sz_ = 0;

  //   return std::make_pair(t_1, t_2);
  // }


  //// QUERY & UPDATE ////

  /**
   * @brief  Computes query on the interval [l, r].
   *         Time: O(log(n))
   *
   * @param  l  Start index
   * @param  r  End index
   */
  value_type query(size_t l, size_t r) {
    auto [left, node_right] = split_node(node_manager_.root(), l);
    auto [node, right] = split_node(node_right, r - l + 1);

    T result = node->result;

    node_manager_.root() = merge_nodes(merge_nodes(left, node), right);

    return result;
  }

  /**
   * @brief  Updates data on the interval [l, r].
   *         Time: O(log(n))
   *
   * @param  l    Start index
   * @param  r    End index
   * @param  upd  Value that should be applied
   */
  void update(size_t l, size_t r, const value_type& upd) requires (is_updatable) {
    auto [left, node_right] = split_node(node_manager_.root(), l);
    auto [node, right] = split_node(node_right, r - l + 1);

    node->promise = upd;

    node_manager_.root() = merge_nodes(merge_nodes(left, node), right);
  }


  //// NAVIGATION ////

  iterator begin() {
    node_pointer node = node_manager_.root();

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