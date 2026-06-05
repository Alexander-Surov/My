#ifndef TRIE
#define TRIE

#include <string_view>
#include "../Graph/Graph.h"
#include <queue>
#include <stack>


namespace my {

////////// TRIE //////////

class Trie {

  struct Vertex;
  struct Iterator;


 public:
  using string_id = uint64_t;

  using iterator = Iterator;


 private:
  using graph_type = Graph<Vertex, void, char>;

  using graph_iterator = typename graph_type::iterator;


 private:
  //// VERTEX ////

  struct Vertex {
    string_id id;
    bool is_terminal = false;
    graph_iterator suff_link;
    graph_iterator exit_link;
  };


  //// ITERATOR ////

  class Iterator {

    friend Trie;


   private:
    graph_iterator vertex_;
    graph_iterator root_;


   public:
    Iterator() = default;


   private:
    Iterator(graph_iterator it, graph_iterator root) : vertex_(it), root_(root) {
    }


   private:
    // Access //

    decltype(auto) operator->() {
      return vertex_.operator->();
    }

    decltype(auto) operator->() const {
      return vertex_.operator->();
    }


    graph_iterator& iter() {
      return vertex_;
    }
    const graph_iterator& iter() const {
      return vertex_;
    }


   public:
    //// ACCESS ////

    bool is_terminal() const {
      return vertex_->is_terminal;
    }

    size_t str_id() const {
      return vertex_->id;
    }

    Iterator suff_link() const {
      return Iterator(vertex_->suff_link, root_);
    }
    Iterator exit_link() const {
      return Iterator(vertex_->exit_link, root_);
    }


    //// ADVANCE ////

    void advance(char c) {
      while (vertex_ != graph_iterator() && !vertex_.contains_ti(c)) {
        vertex_ = vertex_->suff_link;
      }
      vertex_ = (vertex_ != graph_iterator() ? vertex_.to(c) : root_);
    }


    //// EQUALITY OPERSTORS ////

    bool operator==(const Iterator& it) const {
      return vertex_ == it.vertex_;
    }
    bool operator!=(const Iterator& it) const {
      return vertex_ != it.vertex_;
    }
  };


 private:
  graph_type graph_;
  graph_iterator root_;


 public:
  Trie() {
    root_ = graph_.add_vertex(Vertex());
  }


 private:
  // ID generator //

  string_id generate_id() const {
    static string_id id = 0;
    return id++;
  }


 public:
  //// EDITORS ////

  string_id add(const std::string_view& str) {
    graph_iterator it = root_;

    for (char c : str) {
      if (!it.contains_ti(c)) {
        graph_.add_edge(it, graph_.add_vertex(Vertex()), c);
      }

      it = it.to(c);
    }

    it->is_terminal = true;
    it->id = generate_id();

    return it->id;
  }

  void remove(const std::string_view& str) {
    graph_iterator it = root_;
    std::stack<graph_iterator> chain;

    for (char c : str) {
      if (!it.contains_ti(c)) {
        return;
      }

      it = it.to(c);
      chain.push(it);
    }

    it->is_terminal = false;

    if (it.neighbours_num() == 0 && it != root_) {
      graph_.remove_vertex(it);
      chain.pop();

      while (!chain.empty() && chain.top().neighbours_num() == 1) {
        it = chain.top();
        chain.pop();
        graph_.remove_vertex(it);
      }
    }
  }


  //// ACCESSOR ////

  bool contains(const std::string_view& str) const {
    graph_iterator it = root_;

    for (char c : str) {
      if (!it.contains_ti(c)) {
        return false;
      }

      it = it.to(c);
    }

    return it->is_terminal;
  }


  //// LINKS CONSTRUCTOR ////

  void construct_links() {
    std::queue<std::tuple<char, Iterator, Iterator>> queue;

    for (auto&& [_, v] : root_.neighbours()) {
      v->suff_link = root_;

      for (auto&& [ti, u] : v.neighbours()) {
        queue.emplace(ti, Iterator(root_, root_), Iterator(u, root_));
      }
    }

    while (!queue.empty()) {
      auto [c, suff, vertex] = queue.front();
      queue.pop();

      while (suff != Iterator() && !suff.iter().contains_ti(c)) {
        suff = suff.suff_link();
      }

      vertex->suff_link = (suff != Iterator() ? suff.iter().to(c) : root_);
      suff = Iterator(vertex->suff_link, root_);

      if (suff != Iterator() && suff.iter() != root_) {
        vertex->exit_link = (suff.is_terminal() ? suff.iter() : suff->exit_link);
      }

      for (auto&& [ti, v] : vertex.vertex_.neighbours()) {
        queue.emplace(ti, suff, Iterator(v, root_));
      }
    }
  }


  //// ACCESS ////

  Iterator root() const {
    return Iterator(root_, root_);
  }
};

}  // namepsace my

#endif