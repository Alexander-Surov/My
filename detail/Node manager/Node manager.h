#ifndef NODE_MANAGER
#define NODE_MANAGER

#include "util.h"
#include "Nodes.h"
#include "../../Memory/FreeList allocator.h"


namespace my {

template <typename Node>
requires (node_util::BinaryNodeConcept<Node>)
class BinaryNodeManager {
 public:
  using node_type = node::BinaryNode<Node>;

  using pointer = typename node_type::pointer;
  using const_pointer = typename node_type::const_pointer;

  using iterator = node::iterator::BinaryIterator<Node, false>;
  using const_iterator = node::iterator::BinaryIterator<Node, true>;


 private:
  pointer root_ = nullptr;
  FreelistAllocator<node_type> alloc_;


 public:
  BinaryNodeManager(std::pmr::memory_resource* r = std::pmr::get_default_resource()) : alloc_(r) {
  }

  BinaryNodeManager(const BinaryNodeManager& other) : alloc_(other.alloc_) {
    root_ = copy(other.root_);
  }
  BinaryNodeManager(BinaryNodeManager&&) = default;

  BinaryNodeManager& operator=(const BinaryNodeManager& other) {
    alloc_ = other.alloc_;
    root_ = copy(other.root_);
    return *this;
  }
  BinaryNodeManager& operator=(BinaryNodeManager&&) = default;

  ~BinaryNodeManager() {
    del_node(root_);
  }


 public:
  //// ROOT ////

  pointer& root() {
    return root_;
  }
  const_pointer root() const {
    return root_;
  }


  //// COPY FUNCTION ////

  pointer copy(pointer node) {
    if (node == nullptr) {
      return nullptr;
    }

    pointer p = new_node(*static_cast<Node*>(node));

    if (node->left != nullptr) {
      p->left = copy(node->left);
      p->left->parent = p;
    }
    if (node->right != nullptr) {
      p->right = copy(node->right);
      p->right->parent = p;
    }

    return p;
  }
  pointer copy(const_pointer node) {
    if (node == nullptr) {
      return nullptr;
    }

    pointer p = new_node(*static_cast<const Node*>(node));

    if (node->left != nullptr) {
      p->left = copy(node->left);
      p->left->parent = p;
    }
    if (node->right != nullptr) {
      p->right = copy(node->right);
      p->right->parent = p;
    }

    return p;
  }


  //// MERGE ALLOCATORS ////

  void merge_alloc(BinaryNodeManager& other) {
    alloc_.absorb(other.merge_alloc);
  }


  //// ALLOCATION & DEALLOCATION ////

  template <typename... Args>
  pointer new_node(Args&&... args) {
    return std::construct_at(alloc_.allocate(1), std::forward<Args>(args)...);
  }
  void del_node(pointer node) {
    if (node == nullptr) {
      return;
    }

    node::traversal::postorder(node, [&alloc_ = this->alloc_](pointer node) {
      std::destroy_at(node);
      alloc_.deallocate(node, 1);
    });
  }
};


template <typename Node>
requires (node_util::BinomialNodeConcept<Node>)
class BinomialNodeManager {
 public:
  using node_type = node::BinomialNode<Node>;
  using list_type = typename node_type::List;

  using pointer = typename node_type::pointer;
  using const_pointer = typename node_type::const_pointer;

  using iterator = node::iterator::BinomialIterator<Node, false>;
  using const_iterator = node::iterator::BinomialIterator<Node, true>;


 private:
  list_type root_list_;
  FreelistAllocator<node_type> alloc_;


 public:
  BinomialNodeManager(std::pmr::memory_resource* r = std::pmr::get_default_resource()) : alloc_(r) {
  }

  BinomialNodeManager(const BinomialNodeManager& other) : alloc_(other.alloc_) {
    for (auto p = other.root_list_.front; p != nullptr; p = p->next) {
      root_list_.push_back(copy(p));
    }
  }
  BinomialNodeManager(BinomialNodeManager&&) = default;

  BinomialNodeManager& operator=(const BinomialNodeManager& other) {
    alloc_ = other.alloc_;
    for (auto p = other.root_list_.front; p != nullptr; p = p->next) {
      root_list_.push_back(copy(p));
    }
    return *this;
  }
  BinomialNodeManager& operator=(BinomialNodeManager&&) = default;

  ~BinomialNodeManager() {
    for (auto p = root_list_.front; p != nullptr; p = p->next) {
      del_node(p);
    }
  }


 public:
  //// ACCESS ////

  list_type& root_list() {
    return root_list_;
  }
  list_type root_list() const {
    return root_list_;
  }


  //// COPY FUNCTION ////

  pointer copy(pointer node) {
    if (node == nullptr) {
      return nullptr;
    }

    pointer p = new_node(static_cast<Node>(*node));

    for (auto q = node->front_subnode; q != nullptr; q = q->next) {
      p.subnode.push_back(copy(q));
    }
  }
  const_pointer copy(const_pointer node) {
    if (node == nullptr) {
      return nullptr;
    }

    pointer p = new_node(static_cast<const Node>(*node));

    for (auto q = node->front_subnode; q != nullptr; q = q->next) {
      p.subnode.push_back(copy(q));
    }
  }


  //// MERGE ALLOCATORS ////

  void merge_alloc(BinomialNodeManager& other) {
    alloc_.absorb(other.merge_alloc);
  }


  //// ALLOCATION & DEALLOCATION ////

  template <typename... Args>
  pointer new_node(Args&&... args) {
    return std::construct_at(alloc_.allocate(1), std::forward<Args>(args)...);
  }
  void del_node(pointer node) {
    if (node == nullptr) {
      return;
    }

    node::traversal::postorder(node, [&alloc_ = this->alloc_](pointer node) {
      std::destroy_at(node);
      alloc_.deallocate(node, 1);
    });
  }
};

}  // namespace my

#endif