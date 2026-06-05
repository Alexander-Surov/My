#ifndef FARACH_COLTON_BENDER
#define FARACH_COLTON_BENDER

#include "util.h"
#include "Sparse table.h"
#include <cstdint>  // uint16_t
#include <bit>  // popcount


namespace my {

template <typename T>
class RMQ1 {
 public:
  using value_type = T;


 private:
  struct BlockData {
    uint16_t mask;
    int change;
  };


 private:
  SparseTable<int, std::less<int>> st_;
  std::vector<BlockData> blocks_;
  std::vector<std::vector<int>> mask_precalc_;
  size_t n_ = 0;
  size_t b_ = 0;


 public:
  template <typename Cont>
  RMQ1(const Cont& data) {
    // Parameters
    n_ = std::ranges::size(data);
    b_ = std::max(1UL, size_t(std::log2(n_) / 2));

    // Build relation between blocks and their masks
    auto masks = data
                 | std::views::chunk(b_)
                 | std::views::transform([](const auto& chunk) {
                                           return chunk
                                                  | std::views::slide(2)
                                                  | std::views::transform([](const auto& p) {
                                                                            return p[0] < p[1];
                                                                          });
                                         })
                 | std::views::transform([this](const auto& bits) {
                                           return bits_to_mask(bits);
                                         });

    // Build relation between blocks and their changes
    auto change_calc = [this, accumulated = 0, tmp = 0](uint16_t mask) mutable {
                         tmp = accumulated;
                         accumulated += count_change(mask);
                         return tmp;
                       };

    auto diff_calc = [accumulated = 0](const auto& p) mutable {
                       accumulated += (p[0] < p[1] ? +1 : -1);
                       return accumulated;
                     };

    auto changes = std::views::zip_transform([](int outer, int inner) {
                                               return outer + inner;
                                             },
                                             std::views::concat(std::views::single(0),
                                                                data
                                                                | std::views::slide(2)
                                                                | std::views::drop(b_ - 1)
                                                                | std::views::stride(b_)
                                                                | std::views::transform(diff_calc)),
                                             masks
                                             | std::views::transform(change_calc));

    // Blocks
    auto blocks = std::views::zip_transform([](uint16_t mask, int change) {
                                              return BlockData{mask, change};
                                            },
                                            masks, changes);

    blocks_ = std::vector<BlockData>(std::from_range, blocks);

    size_t k = b_ - ((n_ - 1) % b_ + 1);
    blocks_.back().mask <<= k;
    blocks_.back().mask |= uint16_t(std::pow(2, k)) - 1;

    // Calculate all possible masks of size 'b'
    mask_precalc_.reserve(std::pow(2, b_ - 1));

    for (uint16_t mask = 0; mask < std::pow(2, b_ - 1); ++mask) {
      mask_precalc_.push_back(precalculate_table(mask));
    }

    // Build sparse table on blocks
    st_ = SparseTable<int, std::less<int>>(std::views::iota(0UL, blocks_.size())
                                | std::views::transform([this](int i) {
                                    return block_query(i, 0, b_ - 1);
                                  }));
  }


 private:
  // Precalculation //

  uint16_t bits_to_mask(const auto& bits) const {
    uint16_t mask = 0;

    for (size_t i = 0, k = std::ranges::size(bits); i < k; ++i) {
      mask |= bits[k - i - 1] << i;
    }

    return mask;
  }

  int count_change(uint16_t mask) const {
    return 2 * std::popcount(mask) - b_ + 1;
  }

  std::vector<int> precalculate_table(uint16_t mask) {
    std::vector<int> vec(b_ * (b_ + 1) / 2);

    auto nth_bit = [this](uint16_t mask, size_t i) { return bool(mask >> (b_ - i - 1) & 1); };

    // Traversal:
    //　　 ┃ 𝟬　𝟭　𝟮　𝟯
    //　━━━╋━━━━━━━━━━━━
    //　𝟬　┃ 0  4  7  9
    //　𝟭　┃ -  1  5  8
    //　𝟮　┃ -  -  2  6
    //　𝟯　┃ -  -  -  3

    vec[0] = 0;

    for (size_t i = 1; i < b_; ++i) {
      vec[index(i, i)] = vec[index(i - 1, i - 1)] + (nth_bit(mask, i) ? +1 : -1);
    }

    for (size_t k = 1; k < b_; ++k) {
      for (size_t i = 0, j = k; j < b_; ++j, ++i) {
        vec[index(i, j)] = std::min(vec[index(i + 1, j)], vec[index(i, j - 1)]);
      }
    }

    return vec;
  }


  // Helper //

  int block_query(size_t idx, size_t l, size_t r) const {
    int change = mask_precalc_[blocks_[idx].mask][index(l, r)];
    return blocks_[idx].change + change;
  }


  // At //

  /**
   *　　 ┃ 𝟬　𝟭　𝟮　𝟯
   *　━━━╋━━━━━━━━━━━━
   *　𝟬　┃ 0  1  2  3
   *　𝟭　┃ -  4  5  6
   *　𝟮　┃ -  -  7  8
   *　𝟯　┃ -  -  -  9
   */
  size_t index(size_t i, size_t j) const {
    return i * (2 * b_ + 1 - i) / 2 + (j - i);
  }


 public:
  //// QUERY ////

  int query(size_t l, size_t r) const {
    size_t left = l - l % b_ + (b_ - 1);
    size_t right = r - r % b_;

    if (right <= left) {
      return block_query(l / b_, l % b_, r % b_);  // ...] [R...L] [...
    }

    int min = std::min(block_query(l / b_, l % b_, b_ - 1),  // ...L) [...]
                       block_query(r / b_, 0, r % b_));      //       [...] (R...

    return left != right - 1  // ...L] (?) [R...
           ? std::min(min, st_.query((left + 1) / b_, (right - 1) / b_))
           : min;
  }
};

}  // namespace my

#endif