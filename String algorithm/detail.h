#ifndef STR_DETAIL
#define STR_DETAIL

#include <string_view>
#include <vector>
#include <unordered_map>


namespace my::str_detail {

//// SEARCH SUBSTRING ////

// Boyer-Moore //

class BoyerMooreShift {
 private:
  std::unordered_map<char, size_t> bad_symbol;
  std::vector<size_t> good_suffix;


 public:
  BoyerMooreShift(const std::string_view& str) {
    size_t len = str.length();

    // Bad symbol
    for (size_t i = 0; i < len; ++i) {
      bad_symbol[str[i]] = i;
    }

    // Good suffix
    good_suffix.resize(len + 1, 0);
    std::vector<size_t> max_bound(len + 1);

    size_t i = len;
    size_t j = len + 1;

    while (i > 0) {
      while (j <= len && str[i - 1] != str[j - 1]) {
        j = max_bound[j];
      }

      max_bound[i] = j;
      --i;
      --j;

      if (good_suffix[j] == 0) {
        good_suffix[j] = j - i;
      }
    }
  }


 public:
  size_t operator()(size_t i, char c) {
    auto subtract = [](size_t i, size_t x) -> size_t { return (i > x ? i - x : 1); };

    size_t bs = (bad_symbol.contains(c) ? subtract(i, bad_symbol[c]) : i + 1);
    size_t gs = good_suffix[i + 1];

    return std::max(bs, gs);
  }
};


// Rabin-Karp //

/**
 * hashₛ(i) = ∑ᵢ₌₀ʳ⁻¹(s[i]∙pʳ⁻¹⁻ⁱ)  ⇒  hashₛ(l, r) = hash(r) - hash(l)∙pʳ⁻ˡ
 */
class RabinKarpHash {
 private:
  static constexpr size_t m = 1'000'000'007;  // m^2 must not overflow
  static constexpr size_t p = 31;


 private:
  std::vector<size_t> hs;
  std::vector<size_t> deg;


 public:
  RabinKarpHash(const std::string_view& str) {
    size_t len = str.length();

    hs.resize(len + 1, 0);
    deg.resize(len + 1, 1);

    for (size_t i = 0; i < len; ++i) {
      hs[i + 1] = (hs[i] * p + size_t(str[i])) % m;
      deg[i + 1] = (deg[i] * p) % m;
    }
  }


 public:
  size_t operator()(size_t l, size_t r) {
    return (hs[r] + m - (hs[l] * deg[r - l]) % m) % m;
  }
};

}  // namespace my::str_util

#endif