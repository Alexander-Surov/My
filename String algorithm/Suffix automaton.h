#ifndef SUFFIX_AUTOMATON
#define SUFFIX_AUTOMATON

#include <string_view>
#include "../Graph/Graph.h"
#include <unordered_map>
#include <optional>
#include <queue>
#include <algorithm>


namespace my {

////////// SUFFIX AUTOMATON //////////

/**
 * @brief   Suffix automaton of string S is an efficient data structure, which allows the storage, processing, and
 *          retrieval of compressed information about all S's substrings.
 *
 *          The structure was first discovered in 1983 by Anselm Blumer et al., and in 1985 the first linear algorithm
 *          for the construction was presented by Maxime Crochemore and Anselm Blumer.
 */
class SuffixAutomaton {

  struct Vertex;


 private:
  using graph_type = Graph<Vertex, void, char>;

  using vertex_id = graph_type::vertex_id;
  using graph_iterator = graph_type::iterator;


 private:
  //// VERTEX ////

  struct Vertex {
    bool is_terminal = false;
    bool is_clone = false;
    size_t length = 0;
    size_t idx = 0;
    graph_iterator suff_link;
  };


 private:
  graph_type graph_;
  graph_iterator root_;
  graph_iterator last_;

  std::unordered_map<vertex_id, size_t> paths_num_;
  std::unordered_map<vertex_id, std::vector<graph_iterator>> inv_suff_links_;


 public:
  /**
   * @brief  Constructor.
   *
   *         Time complexity: Θ(|S|)
   *         Space complexity: Θ(|S|)
   *
   *         Suffix Automaton has at most 2∙|S| - 1 states and at most 3∙|S| - 4 transitions.
   *
   * @param  str  String to build the SA on.
   */
  explicit SuffixAutomaton(const std::string_view& str) {
    // initial vertex
    last_ = root_ = graph_.add_vertex(Vertex());
    root_->idx = size_t(-1);

    // Iteratively build SA
    for (char c : str) {
      append_symbol(c);
    }

    // Set terminal vertices
    while (last_ != graph_iterator() && last_ != root_) {
      last_->is_terminal = true;
      last_ = last_->suff_link;
    }

    // Preprocessing for the function 'kth'
    paths_num_.reserve(graph_.vertices().size());

    for (auto&& v : graph_.vertices()) {
      paths_num_[v.id()] = 0;
    }

    graph_.DFS(root_, [this](auto&& v, size_t state) {
      if (state == 2) {
        for (auto&& [_, u] : v.neighbours()) {
          paths_num_[v.id()] += paths_num_[u.id()] + 1;
        }
      }
    });

    // Preprocessing for the function 'find_entries'
    for (auto&& v : graph_.vertices()) {
      if (v != root_) {
        inv_suff_links_[v->suff_link.id()].push_back(v);
      }
    }
  }


 private:
  // Clone //

  graph_iterator clone(graph_iterator sample, char c, graph_iterator parent) {
    graph_iterator vertex = graph_.add_vertex(Vertex());

    vertex->is_clone = true;
    vertex->length = parent->length + 1;
    vertex->idx = sample->idx;
    vertex->suff_link = sample->suff_link;

    for (auto&& [c, v] : sample.neighbours()) {
      graph_.add_edge(vertex, v, c);
    }

    sample->suff_link = vertex;

    while (parent != graph_iterator() && parent.to(c) == sample) {
      graph_.add_edge(parent, vertex, c);
      parent = parent->suff_link;
    }

    return vertex;
  }


  // Append symbol //

  void append_symbol(char c) {
    graph_iterator vertex = graph_.add_vertex(Vertex());

    vertex->length = last_->length + 1;
    vertex->idx = last_->idx + 1;

    while (last_ != graph_iterator() && !last_.contains_ti(c)) {
      graph_.add_edge(last_, vertex, c);
      last_ = last_->suff_link;
    }

    if (last_ == graph_iterator()) {
      vertex->suff_link = root_;

    } else if (last_.to(c)->length == last_->length + 1) {
      vertex->suff_link = last_.to(c);

    } else {
      vertex->suff_link = clone(last_.to(c), c, last_);
    }

    last_ = vertex;
  }


  // Find //

  std::optional<graph_iterator> find(const std::string_view& str) const {
    graph_iterator it = root_;

    for (char c : str) {
      if (!it.contains_ti(c)) {
        return std::nullopt;
      }

      it = it.to(c);
    }

    return it;
  }


 public:
  //// QUERIES ////

  /**
   * @brief  Indicates whether `str` is a substring of `S`.
   *         Time complexity: Θ(|P|)
   *
   * @param  str  String to check.
   */
  bool is_substring(const std::string_view& str) const {
    return find(str).has_value();
  }

  /**
   * @brief  Counts the number of unique substrings in `S`.
   *         Time complexity: O(1)
   */
  size_t substrings_num() const {
    // size_t num = 0;
    // for(auto&& v : graph_.vertices()) {
    //   if (v != root_) {
    //     num += v->length - v->suff_link->length;
    //   }
    // }

    return paths_num_.at(root_.id());
  }

  /**
   * @brief  Finds the lexicographically kᵗʰ substring of `S`.
   *         Time complexity: Θ(|ans|)
   *
   * @param  k  [1, substrings_num()] - lexicographical order of the substring.
   */
  std::string kth(size_t k) const {
    if (k < 1 || k > substrings_num()) {
      throw std::out_of_range("k should be in range [1, substrings_num()]");
    }

    std::string str;
    graph_iterator it = root_;

    while (k > 0) {
      auto neighbours = it.neighbours() | std::ranges::to<std::vector>();
      std::ranges::sort(neighbours, [](auto&& l, auto&& r) { return l.first < r.first; });

      for (auto&& [c, v] : neighbours) {
        if (k > paths_num_.at(v.id()) + 1) {
          k -= paths_num_.at(v.id()) + 1;

        } else {
          --k;
          str += c;
          it = v;
          break;
        }
      }
    }

    return str;
  }

  /**
   * @brief  Finds all occurrences of `str` in `S`.
   *         Time complexity: Θ(|P| + |ans|)
   *
   * @param  strs  String to look up.
   */
  std::vector<size_t> find_entries(const std::string_view& str) const {
    auto it = find(str);

    if (!it.has_value()) {
      return {};
    }

    std::vector<size_t> idxs;
    std::queue<graph_iterator> suffs;

    if (!it.value()->is_clone) {
      idxs.push_back(it.value()->idx - str.length() + 1);
    }

    if (inv_suff_links_.contains(it->id())) {
      for (auto&& v : inv_suff_links_.at(it->id())) {
        suffs.push(v);
      }
    }

    while (!suffs.empty()) {
      graph_iterator vertex = suffs.front();
      suffs.pop();

      if (!vertex->is_clone) {
        idxs.push_back(vertex->idx - str.length() + 1);
      }

      if (inv_suff_links_.contains(vertex.id())) {
        for (auto&& v : inv_suff_links_.at(vertex.id())) {
          suffs.push(v);
        }
      }
    }

    return idxs;
  }

  /**
   * @brief  Finds the longest common substring of `S` and `str`.
   *         Time complexity: Θ(|S| + |P|)
   *
   * @param  str  String to search in.
   *
   * @return  [start, end) of LCS in `str`
   */
  std::pair<size_t, size_t> lcs(const std::string_view& str) {
    graph_iterator it = root_;

    size_t length = 0;
		size_t best_length = 0;
    size_t best_idx = 0;

    for (size_t i = 0; i < str.length(); ++i) {
      while (it != root_ && !it.contains_ti(str[i])) {
        it = it->suff_link;
        length = it->length;
      }

      if (it.contains_ti(str[i])) {
        it = it.to(str[i]);
        ++length;
      }

      if (length > best_length) {
        best_length = length;
        best_idx = i;
      }
    }

    return std::make_pair(best_idx - best_length + 1, best_idx + 1);
  }
};

}  // namespace my

#endif