#ifndef SUFFIX_ARRAY
#define SUFFIX_ARRAY

#include <string>
#include <vector>
#include <ranges>
#include <unordered_map>


namespace my {

////////// SUFFIX ARRAY //////////

class SuffixArray {
 private:
  std::string s_;
  std::vector<size_t> sa_;
  std::vector<size_t> r_;
  std::vector<size_t> lcp_;


 public:
  explicit SuffixArray(const std::string_view& str) {
    s_ = str;

    build_sa();
    build_r();
    build_lcp();
  }


 private:
  // Build suffix array //

  void build_sa() {
    sa_.resize(s_.length(), 0);
    // ...
  }


  // Build reversed array //

  void build_r() {
    r_.resize(s_.length(), 0);
    for (size_t i = 0; i < s_.length(); ++i) {
      r_[sa_[i]] = i;
    }
  }


  // Build lcp array //

  void build_lcp() {
    lcp_.resize(s_.length(), 0);

    for (size_t j = 0, k = 0; j < s_.length(); ++j) {
      size_t i = r_[j];

      if (i == 0) {
        k = 0;
        continue;
      }

      if (k > 0) {
        --k;
      }

      while (s_[sa_[i] + k] == s_[sa_[i - 1] + k]) {
        ++k;
      }

      lcp_[i] = k;
    }
  }


  // DC3 //

  std::vector<size_t> dc3(auto idxs) {
    // ...
  }


 public:
  //// QUERIES ////
};

}  // namespace my

#endif