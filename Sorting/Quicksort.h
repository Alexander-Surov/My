#ifndef QUICKSORT
#define QUICKSORT

#include "detail.h"
#include <ranges>


namespace my {

////////// QUICKSORT //////////

/**
 * @brief  Quicksort (Tony Hoare, 1959).
 *
 * @note  The sorting is not stable.
 *
 * @complexity  Time: O(n·log(n)) (avg)
 *                    O(n²) (wc)
 *              Space: O(1)
 *
 * @param  range  The range to sort
 * @param  comp  Comparison function object
 */
template <typename R, typename Comp = std::ranges::less>
void quicksort(R&& range, Comp comp = {}) {
  namespace rng = std::ranges;

  if (rng::size(range) <= 1) {
    return;
  }

  auto pivot = sort_detail::HoarePartition(range, comp);

  quicksort(rng::subrange(rng::begin(range), pivot), comp);
  quicksort(rng::subrange(pivot + 1, rng::end(range)), comp);
}

}  // namespace my

#endif