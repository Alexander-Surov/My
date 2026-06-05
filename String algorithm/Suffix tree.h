#ifndef SUFFIX_TREE
#define SUFFIX_TREE

#include <string_view>
#include "../Graph/Graph.h"
#include <unordered_map>
#include <optional>
#include <algorithm>


namespace my {

////////// SUFFIX TREE //////////

/**
 * @brief   Suffix tree of string S is an efficient data structure, which represents all suffixes of S in a rooted,
 *          directed tree format.
 * 
 *          The concept was first introduced by Peter Weiner in 1973, and in 1976 the first linear algorithm for the
 *          construction was presented by Edward M. McCreight.
 */
class SuffixTree {

  struct Vertex;


 private:
  using graph_type = Graph<Vertex, void, char>;

  using vertex_id = graph_type::vertex_id;
  using graph_iterator = graph_type::iterator;


 private:
  //// VERTEX ////

  struct Vertex {
    bool is_terminal = false;
    graph_iterator suff_link;
    graph_iterator parent;
    size_t depth = 0;
    size_t l = 0;
    size_t r = 0;
  };


 private:
  graph_type graph_;
  graph_iterator root_;
  std::string s_;

  std::unordered_map<vertex_id, size_t> paths_num_;


 public:
  /**
   * @brief  Consuctor. McCreight's algorithm.
   *
   *         Time complexity: Θ(|S|)
   *         Space complexity: Θ(|S|)
   *
   *         On average Suffix tree requires about 20 times as much memory as S.
   *
   * @param  s  String to build the ST on.
   *
   * @note   `str` must not contain symbol '\0'
   */
  explicit SuffixTree(const std::string_view& str) {
    // Add unique character to the end of the string
    s_ = str;
    s_.push_back('\0');

    // Initial vertex
    root_ = graph_.add_vertex(Vertex());
    root_->suff_link = root_->parent = root_;

    // Iteratively add suffixes ST
    graph_iterator head = root_;

    for (size_t i = 0; i < s_.length(); ++i) {
      head = add_suffix(i, head);
    }

    // Remove redundant leaves and identify terminal vertices
    auto vec = graph_.vertices() | std::ranges::to<std::vector>();

    for (auto&& v : vec) {
      if (s_[v->l] == '\0') {
        v->parent->is_terminal = true;
        graph_.remove_edge(v->parent, '\0');
        graph_.remove_vertex(v);

      } else if (s_[v->r] == '\0') {
        v->is_terminal = true;
        --v->r;
        --v->depth;
      }
    }

    root_->is_terminal = false;
    s_.pop_back();

    // Preprocessing for the function 'kth'
    paths_num_.reserve(graph_.vertices().size());

    for (auto&& v : graph_.vertices()) {
      paths_num_[v.id()] = 0;
    }

    graph_.DFS(root_, [this](auto&& v, size_t state) {
      if (state == 2) {
        for (auto&& [_, u] : v.neighbours()) {
          paths_num_[v.id()] += paths_num_[u.id()] + length(u);
        }
      }
    });
  }


 private:
  // Add suffix //

  graph_iterator add_suffix(size_t i, graph_iterator prev_head) {
    graph_iterator suff = find_suffix_link(prev_head);
    graph_iterator head = find_extended_locus(i, suff);
    return head;
  }


  // Locus finders //

  graph_iterator find_suffix_link(graph_iterator head) {
    if (head->suff_link != graph_iterator()) {
      return head->suff_link;
    }

    size_t idx = head->l;
    size_t rest_sybs = length(head);
    graph_iterator vertex = head->parent->suff_link;

    if (head->parent == root_) {
      ++idx;
      --rest_sybs;
    }

    if (rest_sybs > 0 && !vertex.contains_ti(s_[idx])) {
      add_tail(vertex, idx, idx + rest_sybs - 1);
    }

    if (rest_sybs > 0) {
      while (length(vertex.to(s_[idx])) - 1 < rest_sybs) {
        vertex = vertex.to(s_[idx]);
        idx += length(vertex);
        rest_sybs -= length(vertex);

        if (rest_sybs > 0 && !vertex.contains_ti(s_[idx])) {
          vertex = add_tail(vertex, idx, idx + rest_sybs - 1);
          break;
        }
      }
    }

    if (rest_sybs > 0) {
      vertex = cut_edge(vertex.to(s_[idx]), rest_sybs);
    }

    return head->suff_link = vertex;
  }

  graph_iterator find_extended_locus(size_t i, graph_iterator vertex) {
    size_t idx = i + vertex->depth;

    while (vertex.contains_ti(s_[idx])) {
      vertex = vertex.to(s_[idx]);
      size_t pos = vertex->l;

      while (pos <= vertex->r && s_[pos] == s_[idx]) {
        ++pos;
        ++idx;
      }

      if (pos <= vertex->r) {
        vertex = cut_edge(vertex, pos - vertex->l);
        break;
      }
    }

    graph_iterator tail = add_tail(vertex, idx, s_.length() - 1);

    return vertex;
  }


  // Vertex editors //

  graph_iterator cut_edge(graph_iterator vertex, size_t len) {
    graph_iterator mid_vertex = graph_.add_vertex(Vertex());
    graph_iterator parent = vertex->parent;

    graph_.add_edge(parent, mid_vertex, s_[vertex->l]);
    mid_vertex->parent = parent;
    mid_vertex->l = vertex->l;
    mid_vertex->r = vertex->l + len - 1;
    mid_vertex->depth = parent->depth + length(mid_vertex);

    graph_.add_edge(mid_vertex, vertex, s_[mid_vertex->r + 1]);
    vertex->parent = mid_vertex;
    vertex->l = mid_vertex->r + 1;

    return mid_vertex;
  }

  graph_iterator add_tail(graph_iterator vertex, size_t l, size_t r) {
    if (r < l) {
      return graph_iterator();
    }

    graph_iterator tail = graph_.add_vertex(Vertex());

    graph_.add_edge(vertex, tail, s_[l]);
    tail->parent = vertex;
    tail->l = l;
    tail->r = r;
    tail->depth = vertex->depth + length(tail);

    return tail;
  }


  // Length //

  size_t length(graph_iterator it) const {
    return it->r - it->l + 1;
  }


  // Find //

  std::optional<std::pair<graph_iterator, size_t>> find(const std::string_view& str) const {
    graph_iterator it = root_;

    size_t i = 0;
    size_t pos = 0;

    while (i < str.length()) {
      if (!it.contains_ti(str[i])) {
        return std::nullopt;
      }

      it = it.to(str[i]);
      pos = it->l;

      while (pos <= it->r && i < str.length()) {
        if (str[i] != s_[pos]) {
          return std::nullopt;
        }
        ++i;
        ++pos;
      }
    }

    return std::make_pair(it, pos - it->l);
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
        if (k > paths_num_.at(v.id()) + length(v)) {
          k -= paths_num_.at(v.id()) + length(v);

        } else if (k >= length(v)) {
          str += s_.substr(v->l, length(v));
          k -= length(v);
          it = v;
          break;

        } else {
          str += s_.substr(v->l, k);
          k = 0;
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
   * @param  str  String to look up.
   */
  std::vector<size_t> find_entries(const std::string_view& str) const {
    auto it = find(str);

    if (!it.has_value()) {
      return {};
    }

    std::vector<size_t> idxs;

    graph_.DFS(it.value().first, [&idxs, this](auto&& v, size_t state) {
      if (state == 0) {
        if (v->is_terminal) {
          idxs.push_back(s_.length() - v->depth);
        }
      }
    });

    return idxs;
  }
};

}  // namespace my

#endif