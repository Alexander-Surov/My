#ifndef SORT_DETAIL
#define SORT_DETAIL

#include <concepts>
#include <functional>
#include <algorithm>
#include <stdexcept>


namespace my::sort_detail {

////////// CONCEPTS //////////

template <typename R>
concept Range = std::ranges::random_access_range<R>;

template <typename Comp, typename T>
concept Comparator = std::predicate<Comp, T, T>;


////////// HELPER FUNCTIONS //////////

//// QUICKSORT ////

template <typename R, typename Comp>
std::ranges::iterator_t<R> HoarePartition(R&& range, Comp comp) {
  namespace rng = std::ranges;

  auto pivot = rng::begin(range) + rng::size(range) / 2;  // NB: Median of Medians -> 3:7

  auto i = rng::begin(range);
  auto j = rng::begin(range) + rng::size(range) - 1;

  while (rng::distance(i, j) > 0) {
    while (comp(*i, *pivot)) {
      ++i;
    }
    while (comp(*pivot, *j)) {
      --j;
    }

    rng::iter_swap(i, j);

    if (pivot == i) {
      pivot = j;
      ++i;

    } else if (pivot == j) {
      pivot = i;
      --j;

    } else {
      ++i;
      --j;
    }
  }

  return pivot;
}


//// MERGESORT ////

template <typename R, typename Comp>
void Merge(R&& range, const std::ranges::iterator_t<R>& split, Comp comp) {
  namespace rng = std::ranges;

  std::vector<rng::range_value_t<R>> buffer;
  buffer.reserve(rng::size(range));

  auto i = rng::begin(range);
  auto j = split;

  while (i != split && j != rng::end(range)) {
    if (comp(*j, *i)) {
      buffer.push_back(std::move(*j));
      ++j;

    } else {
      buffer.push_back(std::move(*i));
      ++i;
    }
  }

  while (i != split) {
    buffer.push_back(std::move(*i));
    ++i;
  }

  while (j != rng::end(range)) {
    buffer.push_back(std::move(*j));
    ++j;
  }

  rng::move(buffer, rng::begin(range));
}

template <typename R>
void InplaceSwap(R&& range, std::ranges::iterator_t<R> split) {
  namespace rng = std::ranges;

  if (split == rng::begin(range) || split == rng::end(range)) {
    return;
  }

  size_t sz_1 = rng::distance(rng::begin(range), split);
  size_t sz_2 = rng::distance(split, rng::end(range));

  // range reverse
  for (size_t i = 0, rev = rng::size(range) - 1; i < rng::size(range) / 2; ++i) {
    std::swap(range[i], range[rev - i]);
  }

  // 1ˢᵗ subrange reverse
  for (size_t i = 0, rev = sz_2 - 1; i < sz_2 / 2; ++i) {
    std::swap(range[i], range[rev - i]);
  }
  // 2ⁿᵈ subrange reverse
  for (size_t i = 0, rev = rng::size(range) - 1; i < sz_1 / 2; ++i) {
    std::swap(range[sz_2 + i], range[rev - i]);
  }
}

template <typename R, typename Comp>
void InplaceMerge(R&& range, std::ranges::iterator_t<R> split, Comp comp) {
  namespace rng = std::ranges;

  if (split == rng::begin(range) || split == rng::end(range)) {
    return;
  }

  if (rng::size(range) == 2) {
    if (comp(range[1], range[0])) {
      std::swap(range[1], range[0]);
    }
    return;
  }

  size_t sz_1 = rng::distance(rng::begin(range), split);
  size_t sz_2 = rng::distance(split, rng::end(range));

  if (sz_1 > sz_2) {
    auto elem = rng::begin(range) + sz_1 / 2;
    auto lb = rng::lower_bound(rng::subrange(split, rng::end(range)), *elem, comp);

    InplaceSwap(rng::subrange(elem, lb), split);

    split += rng::distance(split, lb) - rng::distance(elem, split);

    InplaceMerge(rng::subrange(rng::begin(range), split), elem, comp);
    InplaceMerge(rng::subrange(split, rng::end(range)), lb, comp);

  } else {
    auto elem = rng::begin(range) + sz_1 + sz_2 / 2;
    auto ub = rng::upper_bound(rng::subrange(rng::begin(range), split), *elem, comp);

    InplaceSwap(rng::subrange(ub, elem), split);

    split += rng::distance(split, elem) - rng::distance(ub, split);

    InplaceMerge(rng::subrange(rng::begin(range), split), ub, comp);
    InplaceMerge(rng::subrange(split, rng::end(range)), elem, comp);
  }
}


//// HEAPSORT ////

// ...

}  // namespace sort_detail

#endif