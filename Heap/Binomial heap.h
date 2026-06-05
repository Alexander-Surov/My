#ifndef BINOMIAL_HEAP
#define BINOMIAL_HEAP

#include "../Node manager/Node manager.h"


namespace my {

/**
 * @brief   Jean Vuillemin (1978).
 *          A data structure that acts as a mergable priority queue.
 *
 * @tparam  T     Type of element
 * @tparam  Comp  Type of comparator (defaults to `std::less`)
 */
template <typename T, typename Comp = std::less<T>>
class BinomialHeap {

  struct Node {
    T value;
  };


 public:
  using value_type = T;

  using reference = T&;
  using const_reference = const T&;

  using value_compare = Comp;

  using iterator = typename BinomialNodeManager<Node>::const_iterator;
  using const_iterator = typename BinomialNodeManager<Node>::const_iterator;


 private:
  using node_type = typename BinomialNodeManager<Node>::node_type;
  using list_type = typename BinomialNodeManager<Node>::list_type;

  using pointer = typename BinomialNodeManager<Node>::pointer;
  using const_pointer = typename BinomialNodeManager<Node>::const_pointer;


 private:
  BinomialNodeManager<Node> node_manager_;
  pointer top_ = nullptr;
  size_t sz_ = 0;


 public:
  /**
   * @brief Constructs an empty heap.
   */
  BinomialHeap() = default;


 private:
  // Super helpers //

  void consolidate(list_type& nodes) {
    pointer own = node_manager_.root_list().front;
    pointer other = nodes.front;

    while (own != nullptr && other != nullptr) {
      if (own->degree < other->degree) {
        own = own->next;

      } else {
        auto other_next = other->next;
        other->parent = nullptr;
        node_manager_.root_list().insert(own, other);
        own = telescope(other);
        other = other_next;
      }
    }

    if (other != nullptr) {
      if (node_manager_.root_list().front != nullptr) {
        node_manager_.root_list().back->next = other;
      } else {
        node_manager_.root_list().front = other;
      }

      other->prev = node_manager_.root_list().back;
      node_manager_.root_list().back = nodes.back;
    }

    nodes = list_type();
  }

  pointer telescope(pointer node) {
    while (node->next != nullptr && node->degree == node->next->degree) {
      // There can be, at most, 3 consecutive nodes of the same degree
      if (node->next->next != nullptr && node->degree == node->next->next->degree) {
        node = node->next;
      }

      if (Comp{}(node->value, node->next->value)) {
        node = node->next;
        node->parent = node->prev;
      } else {
        node->parent = node->next;
      }

      node_manager_.root_list().erase(node);
      node->parent->subnodes.push_back(node);
      ++node->parent->degree;
      node = node->parent;
    }

    return node;
  }


  // Top //

  void find_top() {
    top_ = node_manager_.root_list().front;
    for (auto p = (top_ != nullptr ? top_->next : nullptr); p != nullptr; p = p->next) {
      if (Comp{}(p->value, top_->value)) {
        top_ = p;
      }
    }
  }

  void suggest_top(pointer node) {
    if (top_ == nullptr || Comp{}(node->value, top_->value)) {
      top_ = node;
    }
  }


  // Helpers //

  static void sift_up(pointer node) {
    while (node->parent != nullptr && Comp{}(node->value, node->parent->value)) {
      swap_nodes(node, node->parent);
      node = node->parent;
    }
  }

  static void rise_up(pointer node) {
    while (node->parent != nullptr) {
      swap_nodes(node, node->parent);
      node = node->parent;
    }
  }

  static void swap_nodes(pointer node_1, pointer node_2) {
    if (node_1 == nullptr || node_2 == nullptr) {
      return;
    }

    std::swap(node_1->parent, node_2->parent);

    std::swap(node_1->next != nullptr ? node_1->next->prev : nullptr,
              node_2->next != nullptr ? node_2->next->prev : nullptr);
    std::swap(node_1->next, node_2->next);

    std::swap(node_1->prev != nullptr ? node_1->prev->next : nullptr,
              node_2->prev != nullptr ? node_2->prev->next : nullptr);
    std::swap(node_1->prev, node_2->prev);
  }


 public:
  //// ACCESSORS ////

  /**
   * @brief  Returns a read-only reference to the data at the first element of the heap.
   *         Time: O(1)
   */
  const_reference top() const {
    if (top_ == nullptr) {
      throw heap_util::HeapException("Heap is empty");
    }

    return top_->value;
  }

  /**
   * @brief  Returns the number of elements stored in the heap.
   *         Time: O(1)
   */
  size_t size() const {
    return sz_;
  }

  /**
   * @brief  Tells whether the heap is empty.
   *         Time: O(1)
   */
  bool empty() const {
    return node_manager_.root() == nullptr;
  }


  //// MODIFIERS ////

  /**
   * @brief  Adds data to the heap.
   *         Time: O(log(n))
   *
   * @param  value  Data to be added
   *
   * @return Iterator pointing at the new element.
   */
  template <typename Arg>
  iterator push(Arg&& value) {
    pointer node = node_manager_.new_node(std::forward<Arg>(value));

    suggest_top(node);
    node_manager_.root_list().push_front(node);
    telescope(node);

    ++sz_;

    return iterator(node);
  }

  /**
   * @brief  Removes the element under the iterator.
   *         Time: O(log(n))
   *
   * @param  it  Iterator pointing to the element
   */
  void pop(iterator it) {
    pointer node = static_cast<pointer>(it.operator->());

    rise_up(node);

    node_manager_.root_list().erase(node);
    consolidate(node->subnodes);
    find_top();

    node_manager_.del_node(node);
    --sz_;
  }

  /**
   * @brief  Removes the first element.
   *         Time: O(log(n))
   */
  void extract_top() {
    node_manager_.root_list().erase(top_);
    consolidate(top_->subnodes);

    node_manager_.del_node(top_);
    --sz_;

    find_top();
  }

  /**
   * @brief  Decreases the element by delta.
   *         Time: O(log(n))
   *
   * @param  it     Iterator pointing to the element
   * @param  delta  Value the data should be decreased by
   */
  void decrease_key(iterator it, const_reference delta) {
    pointer node = static_cast<pointer>(it.operator->());

    node->value = node->value - delta;
    sift_up(node);
    suggest_top(node);
  }

  /**
   * @brief  Merges two heaps.
   *         Time: O(log(n))
   *
   * @note   In the end the other heap remains valid but empty.
   *
   * @param  heap  Heap to merge with
   */
  void merge(BinomialHeap& heap) {
    node_manager_.merge(heap.node_manager_);

    suggest_top(heap.top_);
    consolidate(heap.node_manager_.root_list());

    sz_ += heap.sz_;

    heap.top_ = nullptr;
    heap.sz_ = 0;
  }

};

}  // namespace my

#endif