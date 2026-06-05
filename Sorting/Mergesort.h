#ifndef MERGESORT
#define MERGESORT

#include "detail.h"
#include <ranges>


namespace my {

////////// MERGESORT //////////

/**
 * @brief  Mergesort (John von Neumann, 1945).
 *
 * @note  The sorting is stable.
 *
 * @complexity  Time: O(n·log(n))
 *              Space: O(n)
 *
 * @param  range  The range to sort
 * @param  comp  Comparison function object
 */
template <typename R, typename Comp = std::ranges::less>
void mergesort(R&& range, Comp comp = {}) {
  namespace rng = std::ranges;

  if (rng::size(range) <= 1) {
    return;
  }

  auto end = rng::begin(range) + rng::size(range) / 2;

  mergesort(rng::subrange(rng::begin(range), end), comp);
  mergesort(rng::subrange(end, rng::end(range)), comp);

  sort_detail::Merge(range, end, comp);
}


//// IN-PLACE MERGESORT ////

/**
 * @brief  Mergesort (John von Neumann, 1945).
 *
 * @note  The sorting is stable.
 *
 * @complexity  Time: O(n·log²(n))
 *              Space: O(1)
 *
 * @param  range  The range to sort
 * @param  comp  Comparison function object
 */
template <typename R, typename Comp = std::ranges::less>
void inplace_mergesort(R&& range, Comp comp = {}) {
  namespace rng = std::ranges;

  if (rng::size(range) <= 1) {
    return;
  }

  auto end = rng::begin(range) + rng::size(range) / 2;

  inplace_mergesort(rng::subrange(rng::begin(range), end), comp);
  inplace_mergesort(rng::subrange(end, rng::end(range)), comp);

  sort_detail::InplaceMerge(range, end, comp);
}

}  // namespace my

#endif