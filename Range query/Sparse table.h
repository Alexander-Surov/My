#ifndef SPARSE_TABLE
#define SPARSE_TABLE

#include "util.h"
#include <ranges>
#include <vector>
#include <cmath>


namespace my {

template <typename T, typename Qry>
requires (ranqry_util::QryConcept<Qry, T>)
class SparceTableDemo {
 public:
  using value_type = T;
  using qry_type = Qry;


 private:
  std::vector<T> table_;
  std::vector<size_t> idx_inc_;


 public:
  SparceTableDemo() = default;

  template <typename Cont>
  explicit SparceTableDemo(const Cont& data) {
    for (const auto& value : data) {
      table_.push_back(value);
    }
    size_t n = table_.size();
    table_.reserve(size_for(n));

    idx_inc_.reserve(n);
    idx_inc_.push_back(0);

    for (size_t k = 1; pow(2, k) <= n; ++k) {
      idx_inc_.push_back(table_.size());

      for (size_t i = 0; i + pow(2, k) <= n; ++i) {
        table_.push_back(Qry{}(table_at(k - 1, i), table_at(k - 1, i + pow(2, k - 1))));
      }
    }
  }


 private:
  static size_t size_for(size_t n) {
    // Σᵢ₌₀ⁿ ⌊log₂i⌋ = n∙⌊log₂n⌋ - 2^(⌊log₂n⌋ + 1) + ⌊log₂n⌋ + 2
    return n * size_t(log2(n)) - pow(2, size_t(log2(n)) + 1) + size_t(log2(n)) + 2 + n;
  }

  const T& table_at(size_t i, size_t j) const {
    return table_[idx_inc_[i] + j];
  }


 public:
  value_type query(size_t l, size_t r) const {
    size_t k = size_t(log2(r - l + 1));
    return Qry{}(table_at(k, l), table_at(k, r - pow(2, k) + 1));
  }
};


/**
 * @brief   A data structure used for fast queries on a set of static data.
 *
 * @note   This property should be satisfied:
 *         a ⊕ a = a
 *
 * @tparam  T    Type of data
 * @tparam  Qry  Type of query operation (⊕)
 */
template <typename T, typename Qry>
requires (ranqry_util::QryConcept<Qry, T>)
class SparseTable {
 public:
  using value_type = T;
  using qry_type = Qry;


 private:
  std::vector<SparceTableDemo<T, Qry>> tables_;
  SparceTableDemo<T, Qry> sparse_table_;
  size_t n_ = 0;
  size_t b_ = 0;


 public:
  /**
   * @brief  Default constructor.
   */
  SparseTable() = default;

  /**
   * @brief  Constructor.
   *         Complexity: O(n∙log(log(n)))
   *
   * @param  data  Range of elements to build the Sparce table on
   */
  template <typename Cont>
  explicit SparseTable(const Cont& data) : n_(std::ranges::size(data)), b_(std::max(1UL, size_t(log2(n_)))) {
    std::vector<T> subrange_mins;

    subrange_mins.reserve(n_ / b_ + 1);

    for (auto [i, data_subrange] : data | std::views::chunk(b_) | std::views::enumerate) {
      tables_.emplace_back(data_subrange);
      subrange_mins.push_back(tables_.back().query(0, (i < n_ / b_ ? b_ - 1 : n_ % b_ - 1)));
    }

    sparse_table_ = SparceTableDemo<T, Qry>(subrange_mins);
  }


 public:
  //// QUERY ////

  /**
   * @brief  Computes query result on the interval [l, r].
   *         Complexity: O(1)
   *
   * @param  l  Start index
   * @param  r  End index
   */
  value_type query(size_t l, size_t r) const {
    size_t left = l - l % b_ + (b_ - 1);
    size_t right = r - r % b_;

    if (right <= left) {
      return tables_[l / b_].query(l % b_, r % b_);  // ...] [R...L] [...
    }

    T min = std::min(tables_[l / b_].query(l % b_, b_ - 1),  // ...L) [...]
                     tables_[r / b_].query(0, r % b_));      //       [...] (R...

    // ...L] (?) [R...
    return left != right - 1
           ? std::min(min, sparse_table_.query((left + 1) / b_, (right - 1) / b_))
           : min;
  }
};

}  // namespace my

#endif