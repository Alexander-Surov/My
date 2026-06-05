#ifndef SEGMENT_TREE
#define SEGMENT_TREE

#include "util.h"
#include <optional>
#include <cmath>

// TODO:
// (a, n₁) ⊕ (b, n₂) = (a ⊕ b, n₁ + n₂)
// (a, n) ⊙ Δ = (a ⊙ n∙Δ, n)


namespace my {

/**
 * @brief   Jon Bentley (1977).
 *          A data structure that stores information about array intervals as a tree.
 *
 * @note   These properties should be satisfied:
 *         (a ⊕ b) ⊙ c = (a ⊙ c) ⊕ (b ⊙ c)
 *         (a ⊙ b) ⊙ c = a ⊙ (b ⊙ c)
 *
 * @tparam  D    Dimension
 * @tparam  T    Type of data
 * @tparam  Qry  Type for query operation (⊕)
 * @tparam  Upd  Type for update operation (⊙)
 */
template <size_t D, typename T, typename Qry, typename Upd = void>
requires (D > 0 && ranqry_util::QryConcept<Qry, T> && ranqry_util::UpdConcept<Upd, T>)
class MultiSegmentTree {
 public:
  static constexpr size_t dimension = D;
  using value_type = T;
  using qry_type = Qry;
  using upd_type = Upd;


 private:
  friend MultiSegmentTree<D + 1, T, Qry, Upd>;


  struct Node {
    MultiSegmentTree<D - 1, T, Qry, Upd> subtree;
  };


  struct Node1D {
    T value;
    std::optional<T> promise;
  };


 private:
  static constexpr bool is_updatable = !std::same_as<void, Upd>;
  using Nd = std::conditional_t<D >= 2, Node, Node1D>;


 private:
  std::vector<Nd> heap_;
  size_t n_;
  size_t sz_;


 public:
  MultiSegmentTree() = default;

  /**
   * @brief  Constructor.
   *         Complexity: Θ(nᴰ)
   *
   * @param  data  (Multi)-range of elements to build the ST on
   */
  template <typename R>
  explicit MultiSegmentTree(R&& data) : n_(data.size()), sz_(pow(2, ceil(log2(n_)))) {
    heap_.resize(sz_ * 2 - 1);

    for (size_t i = 0; i < n_; ++i) {
      if constexpr (D > 1) {
        heap_[sz_ + i - 1].subtree = MultiSegmentTree<T, D - 1, Qry, Upd>(data[i]);
      } else {
        heap_[sz_ + i - 1].value = data[i];
      }
    }

    for (int i = sz_ - 2; i >= 0; --i) {
      if constexpr (D > 1) {
        heap_[i].subtree = Join(heap_[left(i)].subtree, heap_[right(i)].subtree);
      } else {
        heap_[i].value = Qry{}(heap_[left(i)].value, heap_[right(i)].value);
      }
    }
  }


 private:
  size_t parent(size_t idx) const {
    return (idx - 1) / 2;
  }
  size_t left(size_t idx) const {
    return 2 * idx + 1;
  }
  size_t right(size_t idx) const {
    return 2 * idx + 2;
  }
  bool has_children(size_t idx) const {
    return idx < sz_ - 2;
  }

  void Promise(size_t idx, const T& upd) requires (D == 1) {
    heap_[idx].promise = heap_[idx].promise.has_value()
                         ? Upd{}(heap_[idx].promise.value(), upd)
                         : upd;
  }

  void Push(size_t idx) requires (D == 1) {
    if (heap_[idx].promise.has_value()) {
      if (has_children(idx)) {
        Promise(left(idx), heap_[idx].promise.value());
        Promise(right(idx), heap_[idx].promise.value());
      }

      heap_[idx].value = Upd{}(heap_[idx].value, heap_[idx].promise.value());
      heap_[idx].promise.reset();
    }
  }

  template <class... Args>
  requires (ranqry_util::OrdsConcept<D, Args...>)
  T TopQuery (const std::tuple<Args...>& p_1, const std::tuple<Args...>& p_2,
              size_t idx, size_t l, size_t r, size_t a, size_t b) {
    // [a, b] ⊆ [l, r]
    if (l <= a && b <= r) {
      if constexpr (D > 1) {
        return heap_[idx].subtree.query(metaprog::pop_front(p_1), metaprog::pop_front(p_2));

      } else if constexpr (is_updatable) {
        return heap_[idx].promise.has_value()
               ? Upd{}(heap_[idx].value, heap_[idx].promise.value())
               : heap_[idx].value;

      } else {
        return heap_[idx].value;
      }
    }

    if constexpr (D == 1 && is_updatable) {
      Push(idx);
    }

    // [Left heap bounds] ⋂ [l, r] = ∅
    if ((a + b) / 2 + 1 > r) {
      return TopQuery(p_1, p_2, left(idx), l, r, a, (a + b) / 2);
    }

    // [right heap bounds] ⋂ [l, r] = ∅
    if ((a + b) / 2 < l) {
      return TopQuery(p_1, p_2, right(idx), l, r, (a + b) / 2 + 1, b);
    }

    // [a, b] ⋂ [l, r] ≠ ∅
    return Qry{}(TopQuery(p_1, p_2, left(idx), l, r, a, (a + b) / 2), TopQuery(p_1, p_2, right(idx), l, r, (a + b) / 2 + 1, b));
  }

  template <class... Args>
  requires (is_updatable && ranqry_util::OrdsConcept<D, Args...>)
  void TopUpdate(const std::tuple<Args...>& p_1, const std::tuple<Args...>& p_2, const T& upd,
                 size_t idx, size_t l, size_t r, size_t a, size_t b) {
    // [a, b] ⊆ [l, r]
    if (l <= a && b <= r) {
      if constexpr (D == 1) {
        Promise(idx, upd);
      } else {
        heap_[idx].subtree.Update(metaprog::pop_front(p_1), metaprog::pop_front(p_2), upd);
      }
      return;
    }

    if constexpr (D == 1) {
      Push(idx);
    }

    // [left heap bounds] ⋂ [l, r] ≠ ∅
    if (l <= (a + b) / 2) {
      TopUpdate(p_1, p_2, upd, left(idx), l, r, a, (a + b) / 2);
    }

    // [right heap bounds] ⋂ [l, r] ≠ ∅
    if ((a + b) / 2 + 1 <= r) {
      TopUpdate(p_1, p_2, upd, right(idx), l, r, (a + b) / 2 + 1, b);
    }

    if constexpr (D > 1) {
      heap_[idx].subtree = Join(heap_[left(idx)].subtree, heap_[right(idx)].subtree);
    } else {
      heap_[idx].value = Qry{}(heap_[left(idx)].value, heap_[right(idx)].value);
    }
  }

  template <size_t DD, typename TT, typename QQry, typename UUpd>
  requires (ranqry_util::QryConcept<QQry, TT> && ranqry_util::UpdConcept<UUpd, TT>)
  friend MultiSegmentTree<DD, TT, QQry, UUpd> Join(const MultiSegmentTree<DD, TT, QQry, UUpd>&,
                                                   const MultiSegmentTree<DD, TT, QQry, UUpd>&);


 public:
  //// QUERY ////

  /**
   * @brief  Calculates ⊕-operation result between two points.
   *         Complexity: O(log(n))
   *
   * @param  p_1  Coorditane of the start point
   * @param  p_2  Coorditane of the end point (inclusive)
   */
  template <class... Args>
  requires (ranqry_util::OrdsConcept<D, Args...>)
  value_type query(const std::tuple<Args...>& p_1, const std::tuple<Args...>& p_2) {
    return TopQuery(p_1, p_2, 0, std::get<0>(p_1) + (sz_ - 1), std::get<0>(p_2) + (sz_ - 1), sz_ - 1, sz_ * 2 - 2);
  }
  value_type query(size_t p_1, size_t p_2) requires (D == 1) {
    return query(std::make_tuple(p_1), std::make_tuple(p_2));
  }


  //// MODIFIER ////

  /**
   * @brief  Recalculates tree under ⊙-operation between two points.
   *         Complexity: O(log(n))
   *
   * @param  p_1  Coorditane of the start point
   * @param  p_2  Coorditane of the end point (inclusive)
   */
  template <class... Args>
  requires (is_updatable && ranqry_util::OrdsConcept<D, Args...>)
  void update(const std::tuple<Args...>& p_1, const std::tuple<Args...>& p_2, const value_type& upd) {
    TopUpdate(p_1, p_2, upd, 0, std::get<0>(p_1) + (sz_ - 1), std::get<0>(p_2) + (sz_ - 1), sz_ - 1, sz_ * 2 - 2);
  }
  void update(size_t p_1, size_t p_2, const value_type& upd) requires (D == 1 && is_updatable) {
    update(std::make_tuple(p_1), std::make_tuple(p_2), upd);
  }
};


template <typename T, typename Qry, typename Upd = void>
requires (ranqry_util::QryConcept<Qry, T> && ranqry_util::UpdConcept<Upd, T>)
using SegmentTree = MultiSegmentTree<1, T, Qry, Upd>;


/**
 * @brief  Melds two segment trees into one using ⊕-operation.
 *         Complexity: Θ(n)
 *
 * @param  t_1  First tree
 * @param  t_2  Second tree
 */
template <typename T, size_t D, typename Qry, typename Upd>
requires (ranqry_util::QryConcept<Qry, T> && ranqry_util::UpdConcept<Upd, T>)
MultiSegmentTree<D, T, Qry, Upd> Join(const MultiSegmentTree<D, T, Qry, Upd>& t_1, const MultiSegmentTree<D, T, Qry, Upd>& t_2) {
  if (t_2.heap_.size() > t_1.heap_.size()) {
    throw;  // std::swap(t_1, t_2)
  }

  MultiSegmentTree<T, D, Qry, Upd> tree;

  tree.heap_.reserve(t_1.heap_.size());
  tree.sz_ = t_1.sz_;

  for (size_t i = 0; i < t_2.heap_.size(); ++i) {
    if constexpr (D > 1) {
      tree.heap_.emplace_back(Join(t_1.heap_[i].subtree, t_2.heap_[i].subtree));

    } else {
      std::optional<T> promise;

      switch (t_1.heap_[i].promise.has_value() << 1 | t_2.heap_[i].promise.has_value()) {
        case 0b00:
          break;
        case 0b01:
          promise = t_2.heap_[i].promise.value();
          break;
        case 0b10:
          promise = t_1.heap_[i].promise.value();
          break;
        case 0b11:
          promise = Upd{}(t_1.heap_[i].promise.value(), t_2.heap_[i].promise.value());
          break;
      }

      tree.heap_.emplace_back(Qry{}(t_1.heap_[i].value, t_2.heap_[i].value), promise);
    }
  }

  for (size_t i = t_2.heap_.size(); i < t_1.heap_.size(); ++i) {
    tree.heap_.push_back(t_1.heap_[i]);
  }

  return tree;
}

}  // namespace my

#endif