#ifndef GRAPH
#define GRAPH

#include "../detail/Vertex manager/Vertex manager.h"
#include <queue>
#include <stack>


namespace my {

template <typename Vr = void, typename Ed = void, typename Ti = void, GraphType Gt = GraphType::Directed>
class Graph {
 private:
  using manager = graph_detail::VertexManager<Vr, Ed, Ti, Gt>;

  using pointer = typename manager::pointer;
  using const_pointer = typename manager::const_pointer;

  static constexpr bool is_vr_void = manager::is_vr_void;
  static constexpr bool is_ed_void = manager::is_ed_void;
  static constexpr bool is_ti_void = manager::is_ti_void;

  enum class Colour {
    White,  // вершина ещё не посещена
    Grey,   // из вершины совершается обход
    Black,  // вершина была обнаружена и из неё нет доступных путей
  };


 public:
  static constexpr bool is_directed = manager::is_directed;
  static constexpr bool is_undirected = manager::is_undirected;

  using vertex_type = typename manager::vertex_type;
  using edge_type = typename manager::edge_type;
  using transfer_id = typename manager::transfer_id;

  using vertex_id = typename manager::vertex_id;

  using iterator = typename manager::iterator;
  using const_iterator = typename manager::const_iterator;


 private:
  manager vertex_manager_;


 public:
  Graph() = default;


 public:
  //// ACCESS ////

  iterator vertex(vertex_id id) {
    return vertex_manager_.vertex_iterator(id);
  }
  const_iterator vertex(vertex_id id) const {
    return vertex_manager_.vertex_iterator(id);
  }

  auto vertices() {
    return vertex_manager_.vertices()
           | std::views::transform([this](auto&& p) { return vertex(p.first); });
  }
  auto vertices() const {
    return vertex_manager_.vertices()
           | std::views::transform([this](auto&& p) { return vertex(p.first); });
  }

  bool contains(vertex_id id) const {
    return vertex_manager_.verticies().contains(id);
  }


  //// EDITORS ////

  // Add

  template <typename V>
  iterator add_vertex(V&& vertex) requires (!is_vr_void) {
    return vertex_manager_.vertex_iterator(vertex_manager_.new_vertex(std::forward<V>(vertex)));
  }
  iterator add_vertex() requires (is_vr_void) {
    return vertex_manager_.vertex_iterator(vertex_manager_.new_vertex());
  }

  template <typename From, typename To, typename E>
  requires (!is_ti_void && !is_ed_void)
  void add_edge(From from, To to, transfer_id ti, E&& data) {
    vertex_manager_.new_edge(vertex_manager_.vertex_pointer(from),
                             vertex_manager_.vertex_pointer(to),
                             ti,
                             std::forward<E>(data));
  }

  template <typename From, typename To>
  requires (!is_ti_void && is_ed_void)
  void add_edge(From from, To to, transfer_id ti) {
    vertex_manager_.new_edge(vertex_manager_.vertex_pointer(from),
                             vertex_manager_.vertex_pointer(to),
                             ti);
  }

  template <typename From, typename To, typename E>
  requires (is_ti_void && !is_ed_void)
  void add_edge(From from, To to, E&& data) {
    vertex_manager_.new_edge(vertex_manager_.vertex_pointer(from),
                             vertex_manager_.vertex_pointer(to),
                             std::forward<E>(data));
  }

  template <typename From, typename To>
  requires (is_ti_void && is_ed_void)
  void add_edge(From from, To to) {
    vertex_manager_.new_edge(vertex_manager_.vertex_pointer(from),
                             vertex_manager_.vertex_pointer(to));
  }

  // Remove

  template <typename V>
  void remove_vertex(V v) {
    vertex_manager_.del_vertex(vertex_manager_.vertex_pointer(v));
  }

  template <typename From>
  void remove_edge(From from, transfer_id ti) {
    vertex_manager_.del_edge(vertex_manager_.vertex_pointer(from), ti);
  }


  //// ALGORITHMS ////

  template <typename Visitor>
  void BFS(iterator start, Visitor visitor) {
  }

  template <typename Visitor>
  void DFS(iterator start, Visitor visitor) {
    // Recursion imitation
    std::stack<std::tuple<iterator, std::vector<iterator>>> tour;
    tour.push(std::make_tuple(start, std::vector<iterator>()));

    // Whitening
    std::unordered_map<vertex_id, Colour> colours;

    for (auto&& v : vertices()) {
      colours[v.id()] = Colour::White;
    }

    // Time logging
    std::unordered_map<vertex_id, size_t> time_in;
    std::unordered_map<vertex_id, size_t> time_out;
    size_t time = 0;

    // Processing
    while (!tour.empty()) {
      auto& [v, subs] = tour.top();

      // The first meeting
      if (colours[v.id()] == Colour::White) {
        visitor(v, 0);

        colours[v.id()] = Colour::Grey;
        time_in[v.id()] = time++;

        for (auto&& [_, u] : v.neighbours()) {
          if (colours[u.id()] == Colour::White) {
            subs.push_back(u);
          }
        }

        if (!subs.empty()) {
          tour.push(std::make_tuple(subs.back(), std::vector<iterator>()));
          subs.pop_back();
        }

      // Process other subvertices
      } else if (!subs.empty()) {
        while (!subs.empty() && colours[subs.back().id()] != Colour::White) {
          subs.pop_back();
        }

        if (!subs.empty()) {
          visitor(v, 1);

          tour.push(std::make_tuple(subs.back(), std::vector<iterator>()));
          subs.pop_back();
        }

      // The last meeting
      } else {
        visitor(v, 2);

        colours[v.id()] = Colour::Black;
        time_out[v.id()] = time++;

        tour.pop();
      }
    }
  }
  template <typename Visitor>
  void DFS(const_iterator start, Visitor visitor) const {
    // Recursion imitation
    std::stack<std::tuple<iterator, std::vector<iterator>>> tour;
    tour.push(std::make_tuple(start, std::vector<iterator>()));

    // Whitening
    std::unordered_map<vertex_id, Colour> colours;

    for (auto&& v : vertices()) {
      colours[v.id()] = Colour::White;
    }

    // Time logging
    std::unordered_map<vertex_id, size_t> time_in;
    std::unordered_map<vertex_id, size_t> time_out;
    size_t time = 0;

    // Processing
    while (!tour.empty()) {
      auto& [v, subs] = tour.top();

      // The first meeting
      if (colours[v.id()] == Colour::White) {
        visitor(v, 0);

        colours[v.id()] = Colour::Grey;
        time_in[v.id()] = time++;

        for (auto&& [_, u] : v.neighbours()) {
          if (colours[u.id()] == Colour::White) {
            subs.push_back(u);
          }
        }

        if (!subs.empty()) {
          tour.push(std::make_tuple(subs.back(), std::vector<iterator>()));
          subs.pop_back();
        }

      // Process other subvertices
      } else if (!subs.empty()) {
        while (!subs.empty() && colours[subs.back().id()] != Colour::White) {
          subs.pop_back();
        }

        if (!subs.empty()) {
          visitor(v, 1);

          tour.push(std::make_tuple(subs.back(), std::vector<iterator>()));
          subs.pop_back();
        }

      // The last meeting
      } else {
        visitor(v, 2);

        colours[v.id()] = Colour::Black;
        time_out[v.id()] = time++;

        tour.pop();
      }
    }
  }
};

} // namespace my

#endif