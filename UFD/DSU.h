#ifndef DISJOINT_SET_UNION
#define DISJOINT_SET_UNION

#include <vector>

namespace my {

class DSU {
 private:
  std::vector<size_t> parent_;
  std::vector<size_t> rank_;


 public:
  explicit DSU(size_t n) : parent_(n), rank_(n, 0) {
    for (size_t i = 0; i < n; ++i) {
      parent_[i] = i;
    }
  }


 public:
  size_t find(size_t x) {
    return (x == parent_[x] ? x : parent_[x] = find(parent_[x]));
  }

  void join(size_t x, size_t y) {
    x = find(x);
    y = find(y);

    if (rank_[x] < rank_[y]) {
      parent_[x] = y;

    } else if (rank_[x] > rank_[y]) {
      parent_[y] = x;

    } else {
      parent_[x] = y;
      ++rank_[y];
    }
  }
};

}  // namespace my

#endif