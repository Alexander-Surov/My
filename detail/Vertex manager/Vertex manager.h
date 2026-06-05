#ifndef VERTEX_MANAGER
#define VERTEX_MANAGER

#include "../Memory/FreeList allocator.h"
#include <unordered_map>
#include <ranges>


namespace my {

////////// ... //////////

enum class GraphType {
  Directed,
  Undirected
};

}  // namespace my


namespace my::graph_detail {

////////// EDGE & VERTEX //////////

template <typename M>
struct Edge {
  M::pointer end = nullptr;
  M::edge_type* data = nullptr;
};


template <typename M>
struct Vertex {
  std::unordered_map<typename M::transfer_id, Edge<M>> transfer;
  M::vertex_id id = -1;
  M::vertex_type* data = nullptr;
};


////////// ITERATOR //////////

/**
 * @brief  
 */
template <typename M, bool is_const>
class Iterator {

  friend M;
  friend Iterator<M, true>;


 private:
  M::pointer vert_ = nullptr;


 public:
  Iterator() = default;

  Iterator(const Iterator<M, false>& other) : vert_(other.vert_) {
  }


 private:
  Iterator(M::const_pointer p) : vert_(const_cast<typename M::pointer>(p)) {
  }


 public:
  //// ACCESS ////

  M::vertex_id id() const {
    return vert_->id;
  }

  decltype(auto) operator->() requires(!is_const && !M::is_vr_void) {
    return vert_->data;
  }
  decltype(auto) operator->() const requires(!M::is_vr_void) {
    return vert_->data;
  }

  decltype(auto) edge(M::transfer_id ti) requires(!is_const && !M::is_ed_void) {
    return *vert_->transfer[ti].data;
  }
  decltype(auto) edge(M::transfer_id ti) const requires(!M::is_ed_void) {
    return *vert_->transfer[ti].data;
  }

  auto neighbours() requires(!is_const) {
    return vert_->transfer
           | std::views::transform([](auto&& p) { return std::make_pair(p.first, Iterator(p.second.end)); });
  }
  auto neighbours() const {
    return vert_->transfer
           | std::views::transform([](auto&& p) { return std::make_pair(p.first, Iterator(p.second.end)); });
  }

  size_t neighbours_num() const {
    return vert_->transfer.size();
  }

  bool contains_ti(M::transfer_id ti) const {
    return vert_->transfer.contains(ti);
  }


  //// TO ////

  Iterator to(M::transfer_id ti) requires(!is_const) {
    return Iterator(vert_->transfer.at(ti).end);
  }
  Iterator to(M::transfer_id ti) const {
    return Iterator(vert_->tranfer.at(ti).end);
  }


  //// EQUALITY OPERATORS ////

  bool operator==(const Iterator& it) const {
    return vert_ == it.vert_;
  }
  bool operator!=(const Iterator& it) const {
    return vert_ != it.vert_;
  }
};


////////// VERTEX MANAGER //////////

template <typename Vr, typename Ed, typename Ti, GraphType Gt>
class VertexManager {
 public:
  static constexpr bool is_vr_void = std::is_void_v<std::remove_cv_t<Vr>>;
  static constexpr bool is_ed_void = std::is_void_v<std::remove_cv_t<Ed>>;
  static constexpr bool is_ti_void = std::is_void_v<std::remove_cv_t<Ti>>;

  static constexpr bool is_directed = (Gt == GraphType::Directed);
  static constexpr bool is_undirected = (Gt == GraphType::Undirected);

  using vertex_id = uint64_t;
  using transfer_id = std::conditional_t<is_ti_void, vertex_id, Ti>;

  using vertex_type = Vr;
  using edge_type = Ed;

  using pointer = Vertex<VertexManager>*;
  using const_pointer = const Vertex<VertexManager>*;

  using iterator = Iterator<VertexManager, false>;
  using const_iterator = Iterator<VertexManager, true>;


 private:
  std::unordered_map<vertex_id, Vertex<VertexManager>> vertices_;

  FreelistAllocator<std::conditional_t<!is_vr_void, Vr, std::false_type>> v_alloc_;
  FreelistAllocator<std::conditional_t<!is_ed_void, Ed, std::false_type>> e_alloc_;


 public:
  VertexManager(std::pmr::memory_resource* r = std::pmr::get_default_resource()) : v_alloc_(r), e_alloc_(r) {
  }

  VertexManager(const VertexManager& other) : v_alloc_(other.v_alloc_), e_alloc_(other.e_alloc_) {
    for (const auto& [id, ptr] : other.vertices_) {
      if (!vertices_.contains(id)) {
        vertices_[id] = copy(ptr);
      }
    }
  }
  VertexManager(VertexManager&&) = default;

  VertexManager& operator=(const VertexManager& other) {
    v_alloc_ = other.v_alloc_;
    e_alloc_ = other.e_alloc_;

    for (const auto& [id, ptr] : other.vertices_) {
      if (!vertices_.contains(id)) {
        vertices_[id] = copy(ptr);
      }
    }

    return *this;
  }
  VertexManager& operator=(VertexManager&&) = default;

  ~VertexManager() {
    auto verts = vertices_
                 | std::views::transform([](auto&& p) { return std::addressof(p.second); })
                 | std::ranges::to<std::vector>();

    for (auto ptr : verts) {
      del_vertex(ptr);
    }
  }


 private:
  // ID generator //

  vertex_id generate_id() {
    static size_t id = 0;
    return id++;
  }


 public:
  //// ACCESS ////

  pointer vertex_pointer(vertex_id id) {
    return std::addressof(vertices_.at(id));
  }
  const_pointer vertex_pointer(vertex_id id) const {
    return std::addressof(vertices_.at(id));
  }
  pointer vertex_pointer(iterator it) const {
    return it.vert_;
  }
  const_pointer vertex_pointer(const_iterator it) const {
    return it.vert_;
  }

  iterator vertex_iterator(vertex_id id) {
    return iterator(vertex_pointer(id));
  }
  const_iterator vertex_iterator(vertex_id id) const {
    return const_iterator(vertex_pointer(id));
  }
  iterator vertex_iterator(pointer vert) const {
    return iterator(vert);
  }
  const_iterator vertex_iterator(const_pointer vert) const {
    return const_iterator(vert);
  }

  decltype(auto) vertices() {
    return vertices_;
  }
  decltype(auto) vertices() const {
    return vertices_;
  }


  //// COPY FUNCTION ////

  pointer copy(pointer vert) {
    if (vertices_.contains(vert->id)) {
      return vert;
    }

    auto it = (is_vr_void ? new_vertex() : new_vertex(*vert->data));

    for (const auto& [ti, ed] : vert->transfer) {
      if (!vertices_.contains(ed.first->id)) {
        vertices_[ed.first->id] = copy(ed.first);
      }

      if constexpr (is_ti_void) {
        add_edge(vert->id, ti, *ed.second);
      } else {
        add_edge(vert->id, ed.first->id, ti, *ed.second);
      }
    }
  }


  //// ALLOCATION & DEALLOCATION ////

  // New

  template <typename V>
  pointer new_vertex(V&& v) requires(!is_vr_void) {
    auto ptr = std::construct_at(v_alloc_.allocate(1), std::forward<V>(v));
    vertex_id id = generate_id();
    vertices_[id].id = id;
    vertices_[id].data = ptr;
    return vertex_pointer(id);
  }
  pointer new_vertex() requires(is_vr_void) {
    vertex_id id = generate_id();
    vertices_[id].id = id;
    vertices_[id].data = nullptr;
    return vertex_pointer(id);
  }

  template <typename E>
  void new_edge(const_pointer from, const_pointer to, transfer_id ti, E&& data) requires(!is_ti_void && !is_ed_void) {
    if (from->transfer.contains(ti)) {
      del_edge(from, ti);
    }

    auto p = std::construct_at(e_alloc_.allocate(1), std::forward<E>(data));

    pointer f = const_cast<pointer>(from);
    pointer t = const_cast<pointer>(to);

    f->transfer[ti].end = t;
    f->transfer[ti].data = p;

    if constexpr (is_undirected) {
      t->transfer[ti].end = f;
      t->transfer[ti].data = p;
    }
  }
  void new_edge(const_pointer from, const_pointer to, transfer_id ti) requires(!is_ti_void && is_ed_void) {
    if (from->transfer.contains(ti)) {
      del_edge(from, ti);
    }

    pointer f = const_cast<pointer>(from);
    pointer t = const_cast<pointer>(to);

    f->transfer[ti].end = t;

    if constexpr (is_undirected) {
      t->transfer[ti].end = f;
    }
  }
  template <typename E>
  void new_edge(const_pointer from, const_pointer to, E&& data) requires(is_ti_void && !is_ed_void) {
    if (from->transfer.contains(to->id)) {
      del_edge(from, to->id);
    }

    auto p = std::construct_at(e_alloc_.allocate(1), std::forward<E>(data));

    pointer f = const_cast<pointer>(from);
    pointer t = const_cast<pointer>(to);

    f->transfer[t->id].end = t;
    f->transfer[t->id].data = p;

    if constexpr (is_undirected) {
      t->transfer[f->id].end = f;
      t->transfer[f->id].data = p;
    }
  }
  void new_edge(const_pointer from, const_pointer to) requires(is_ti_void && is_ed_void) {
    if (from->transfer.contains(to->id)) {
      del_edge(from, to->id);
    }

    pointer f = const_cast<pointer>(from);
    pointer t = const_cast<pointer>(to);

    f->transfer[t->id].end = t;

    if constexpr (is_undirected) {
      t->transfer[f->id].end = f;
    }
  }

  // Delete

  void del_vertex(const_pointer vert) {
    if (vert == nullptr) {
      return;
    }

    pointer vr = const_cast<pointer>(vert);

    auto verts = vr->transfer
                 | std::views::transform([](auto&& p) { return p.first; })
                 | std::ranges::to<std::vector>();

    for (auto&& ti : verts) {
      del_edge(vr, ti);
    }

    if constexpr (!is_vr_void) {
      std::destroy_at(vr->data);
      v_alloc_.deallocate(vr->data, 1);
    }

    vertices_.erase(vr->id);
  }

  void del_edge(const_pointer from, transfer_id ti) {
    pointer f = const_cast<pointer>(from);
    auto& edge = f->transfer[ti];

    if constexpr (is_undirected) {
      edge.end->transfer.erase(ti);
    }

    if constexpr (!is_ed_void) {
      std::destroy_at(edge.data);
      e_alloc_.deallocate(edge.data, 1);
    }

    f->transfer.erase(ti);
  }
};

}  // namespace my::graph_detail

#endif