#ifndef AVL_TREE
#define AVL_TREE

#include "../detail/Node manager/Node manager.h"


namespace my {

/**
 * @brief   AVL Tree (Georgy Adelson-Velsky and Evgenii Landis, 1962).
 *          AVL tree is a self-balancing binary search tree where the difference between heights of left and right
 *          subtrees for any node cannot be more than one.
 *
 * @tparam  T
 * @tparam  Comp
 */
template <typename T, typename Comp = std::less<T>>
class AVL {

  struct Node {
    T value;
    short height = 0;

   public:
    template <typename Arg>
    Node(Arg&& arg) : value(std::forward<Arg>(arg)) {
    }
  };


 private:
  using manager = BinaryNodeManager<Node>;

  using node_pointer = typename manager::pointer;
  using const_node_pointer = typename manager::const_pointer;


 public:
  using value_type = T;
  using value_compare = Comp;

  using reference = T&;
  using const_reference = const T&;

  using iterator = manager::const_iterator;


 private:
  manager node_manager_;
  Comp comp_;
  size_t sz_ = 0;


 public:
  AVL(Comp comp = {}) : comp_(comp) {
  }


 private:
  // Balance factor //

  short balance_factor(node_pointer node) const {
    if (node == nullptr) {
      return 0;
    }

    return (node->right != nullptr ? node->right->height : -1) -
           (node->left != nullptr ? node->left->height : -1);
  }


  // Height correction //

  void update_height(node_pointer node) {
    short l = (node->left != nullptr ? node->left->height : -1);
    short r = (node->right != nullptr ? node->right->height : -1);
    node->height = std::max(l, r) + 1;
  }


  // Rotation //

  void rotate_left(node_pointer node) {
    node::rotate::rotate(node->right);
    update_height(node);
    update_height(node->parent);
  }

  void rotate_right(node_pointer node) {
    node::rotate::rotate(node->left);
    update_height(node);
    update_height(node->parent);
  }


  // Balance //

  node_pointer balance(node_pointer node) {
    update_height(node);

    if (balance_factor(node) == 2) {
      if (balance_factor(node->right) == -1) {
        rotate_right(node->right);
      }
      rotate_left(node);
      return node->parent;
    }

    if (balance_factor(node) == -2) {
      if (balance_factor(node->left) == 1) {
        rotate_left(node->left);
      }
      rotate_right(node);
      return node->parent;
    }

    return node;
  }

  node_pointer balance_up(node_pointer node) {
    node_pointer top = node;

    while (node != nullptr) {
      top = balance(node);
      node = top->parent;
    }

    return top;
  }



  // Merge //

  node_pointer merge_nodes(node_pointer node_1, node_pointer node_2) {
    if (node_1 == nullptr) {
			return node_2;
    }
		if (node_2 == nullptr) {
			return node_1;
    }

    if (node_2->height < node_1->height) {
      std::swap(node_1, node_2);
    }

    node_1->parent = node_2->parent = nullptr;

    node_pointer rightmost = node_1;

    while (rightmost->right != nullptr) {
      rightmost = rightmost->right;
    }

    if (rightmost->parent != nullptr) {
      rightmost->parent->right = nullptr;
    }

    node_1 = balance_up(rightmost->parent);

    node_pointer node = node_2;

    while (node->height > node_1->height) {  // error
      node = node->left;
    }

    rightmost->left = node_1;
    rightmost->right = node->left;
    node_1->parent = node->left->parent = rightmost;

    node->left = rightmost;
    rightmost->parent = node->left;

    return balance_up(rightmost);
  }


  // Get //

  node_pointer get(const T& value) {
    node_pointer node = node_manager_.root();

    while (node != nullptr) {
      if (comp_(node->value, value)) {
        node = node->right;
      } else if (comp_(value, node->value)) {
        node = node->left;
      } else {
        break;
      }
    }

    return node;
  }


 public:
  //// MODIFIERS ////

  template <typename U>
  void insert(U&& value) {
    node_pointer node = node_manager_.root();
    node_pointer prev = node;
    bool is_last_left;

    while (node != nullptr) {
      prev = node;

      if (comp_(value, node->value)) {
        node = node->left;
        is_last_left = true;

      } else if (comp_(node->value, value)) {
        node = node->right;
        is_last_left = false;

      } else {
        throw std::invalid_argument("Such value already exists in AVL");
      }
    }

    node_pointer new_node = node_manager_.new_node(std::forward<U>(value));
    node = prev;
    ++sz_;

    if (node != nullptr) {
      (is_last_left ? node->left : node->right) = new_node;
      new_node->parent = node;
      node_manager_.root() = balance_up(node);

    } else {
      node_manager_.root() = new_node;
    }
  }

  void erase(const value_type& value) {
    node_pointer node = get(value);

    if (node == nullptr) {
      throw std::invalid_argument("There is no such value in AVL");
    }

    node_pointer top = merge_nodes(node->left, node->right);

    if (node != node_manager_.root()) {
      (node == node->parent->left ? node->parent->left : node->parent->right) = top;
      if (top != nullptr) {
        top->parent = node->parent;
      }

      node_manager_.root() = balance_up(node->parent);

    } else {
      node_manager_.root() = top;
    }

    node->left = node->right = nullptr;
    node_manager_.del_node(node);
    --sz_;
  }

  void merge(AVL& other) {
    node_manager_.merge_alloc(other);
    node_manager_.root() = merge_nodes(node_manager_.root(), other.node_manager_.root());
    other.node_manager_.root() = nullptr;
    sz_ += other.sz_;
    other.sz_ = 0;
  }


  //// LOOKUP ////

  iterator find(const value_type& value) const {
    return iterator(get(value));
  }

  bool contains(const value_type& value) const {
    return get(value) != nullptr;
  }

  iterator lower_bound(const value_type& value) const {
    node_pointer node = node_manager_.root();

    while (node != nullptr) {
      if (comp_(node->value, value)) {
        node = node->right;
      } else if (comp_(value, node->value)) {
        node = node->left;
      } else {
        break;
      }
    }

    // ...
  }
  iterator upper_bound(const value_type& value) const {
  }

  const_reference min() const {
    if (empty()) {
      throw std::logic_error("AVL is empty");
    }

    node_pointer min = node_manager_.root();

    while (min->left != nullptr) {
      min = min->left;
    }

    return min->value;
  }
  const_reference max() const {
    if (empty()) {
      throw std::logic_error("AVL is empty");
    }

    node_pointer max = node_manager_.root();

    while (max->right != nullptr) {
      max = max->right;
    }

    return max->value;
  }


  //// CAPACITY ////

  size_t size() const noexcept {
    return sz_;
  }

  bool empty() const {
    return sz_ == 0;
  }

  node_pointer root() {
    return node_manager_.root();
  }
};

}  // namespace my

#endif