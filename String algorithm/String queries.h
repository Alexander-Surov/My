#ifndef SEARCH_SUBSTRING
#define SEARCH_SUBSTRING

#include "detail.h"
#include "z_pi_S.h"
#include "Trie.h"
#include <unordered_map>


namespace my {

/***/
std::vector<size_t> KnuthMorrisPratt(const std::string_view& text, const std::string_view& pattern) {
  std::vector<size_t> idxs;

  // 𝝅(P)
  std::vector<size_t> pi = PrefixFunction(pattern);

  // KMP on pseudo 𝝅(P#S)
  for (size_t i = 0, k = 0, prev = 0; i < text.length(); ++i) {
    k = prev;

    while (k > 0 && (k < pattern.length() ? text[i] != pattern[k] : true)) {
      k = pi[k - 1];
    }

    prev = (text[i] == pattern[k] ? k + 1 : 0);

    if (prev == pattern.length()) {
      idxs.push_back(i - pattern.length() + 1);
    }
  }

  return idxs;
}

/***/
std::vector<size_t> BoyerMoore(const std::string_view& text, const std::string_view& pattern) {
  std::vector<size_t> idxs;

  str_detail::BoyerMooreShift shift(pattern);

  for (size_t r = pattern.length() - 1; r < text.length();) {
    size_t i = r;
    size_t k = pattern.length() - 1;

    while (text[i] == pattern[k]) {
      if (k == 0) {
        idxs.push_back(i);
        break;
      }
      --i;
      --k;
    }

    r += shift(k, text[i]);
  }

  return idxs;
}

/***/
std::vector<size_t> RabinKarp(const std::string_view& text, const std::string_view& pattern) {
  std::vector<size_t> idxs;

  size_t t_len = text.length();
  size_t p_len = pattern.length();

  str_detail::RabinKarpHash hash(text);
  size_t pattern_hash = str_detail::RabinKarpHash(pattern)(0, p_len);

  auto is_equal_substr = [text, pattern](size_t idx) {
    for (size_t i = 0; i < pattern.length(); ++i) {
      if (pattern[i] != text[idx + i]) {
        return false;
      }
    }
    return true;
  };

  for (size_t i = 0; i < t_len - p_len + 1; ++i) {
    if (hash(i, i + p_len) == pattern_hash && is_equal_substr(i)) {
      idxs.push_back(i);
    }
  }

  return idxs;
}

/***/
std::vector<std::vector<size_t>> AhoCorasick(const std::string_view& text, const std::vector<std::string>& patterns) {
  std::vector<std::vector<size_t>> idxs(patterns.size());

  Trie trie;

  std::unordered_map<Trie::string_id, size_t> id_to_n;
  size_t n = 0;

  for (auto&& str : patterns) {
    auto id = trie.add(str);
    id_to_n[id] = n++;
  }

  trie.construct_links();

  auto it = trie.root();

  for (size_t i = 0; i < text.length(); ++i) {
    it.advance(text[i]);

    if (it.is_terminal()) {
      idxs[id_to_n[it.str_id()]].push_back(i - patterns[it.str_id()].length() + 1);
    }

    auto exit = it.exit_link();

    while (exit != Trie::iterator()) {
      idxs[id_to_n[exit.str_id()]].push_back(i - patterns[id_to_n[exit.str_id()]].length() + 1);
      exit = exit.exit_link();
    }
  }

  return idxs;
}

}  // namespace my

#endif