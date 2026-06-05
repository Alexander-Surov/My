#ifndef Z_PI_S
#define Z_PI_S

#include <string>
#include <vector>
#include <set>


namespace my {

/**
 * @brief  An array 𝝅 of length n, where 𝝅[i] is the length of the longest proper prefix of the substring s[0...i],
 *         which is also a suffix of this substring. A proper prefix of a string is a prefix that is not equal to
 *         the string itself. By definition, 𝝅[0] = 0.
 *
 *         𝝅[i] = max{k : s[0...k-1] = s[i-k+1...i]}
 *
 *         Time: Θ(n)
 */
std::vector<size_t> PrefixFunction(const std::string_view& str) {
  std::vector<size_t> 𝝅(str.length(), 0);

  for (size_t i = 1; i < str.length(); ++i) {
    size_t k = 𝝅[i - 1];

    while (k > 0 && str[i] != str[k]) {
      k = 𝝅[k - 1];
    }

    if (str[i] == str[k]) {
      𝝅[i] = k + 1;
    }
  }

  return 𝝅;
}

/**
 * @brief  An array z of length n where the i-th element is equal to the greatest number of characters starting
 *         from the position i that coincide with the first characters of s.
 *
 *         z[i] = max{k : s[0...k-1] = s[i...i-k+1]}
 *
 *         Time: Θ(n)
 */
std::vector<size_t> ZetaFunction(const std::string_view& str) {
  std::vector<size_t> z(str.length(), 0);
  z[0] = str.length();

  size_t left = 0;
  size_t right = 0;

  for (size_t i = 1; i < str.length(); ++i) {
    if (i < right) {
      z[i] = std::min(z[i - left], right - i);
    }

    while (i + z[i] < str.length() && str[z[i]] == str[i + z[i]]) {
      ++z[i];
    }

    if (right < i + z[i]) {
      left = i;
      right = i + z[i];
    }
  }

  return z;
}


/**
 * @brief  Constracts z-function from the given 𝝅-function.
 *         Time: Θ(n)
 */
std::vector<size_t> PiToZ(const std::vector<size_t>& 𝝅) {
  std::vector<size_t> z(𝝅.size(), 0);

  for (size_t i = 0; i < 𝝅.size(); ++i) {
    if (𝝅[i] > 0) {
      z[i - 𝝅[i] + 1] = 𝝅[i];
    }
  }

  z[0] = 𝝅.size();

  for (size_t i = 0, t = 0; i < 𝝅.size(); i = t + 1, t = i) {
    if (z[i] > 0) {
      for (size_t j = 1; j < z[i]; ++j) {
        if (z[i + j] > z[i]) {
          break;
        }

        z[i + j] = std::min(z[j], z[i] - j);
        t = i + j;
      }
    }
  }

  return z;
}

/**
 * @brief  Constracts 𝝅-function from the given z-function.
 *         Time: Θ(n)
 */
std::vector<size_t> ZToPi(const std::vector<size_t>& z) {
  std::vector<size_t> 𝝅(z.size(), 0);

  for (size_t i = 0; i < z.size(); ++i) {
    for (long delta = z[i] - 1; delta >= 0; --delta) {
      if (𝝅[i + delta] > 0) {
        break;
      }

      𝝅[i + delta] = delta + 1;
    }
  }

  return 𝝅;
}


/**
 * @brief  Constracts minimal lexicographically string from the given 𝝅-function.
 *         Time: Θ(n)
 */
std::string PiToS(const std::vector<size_t>& 𝝅) {
  std::string str;
  str.reserve(𝝅.size());

  str += 'a';

  for (size_t i = 1; i < 𝝅.size(); ++i) {
    if (𝝅[i] > 0) {
      str += str[𝝅[i] - 1];

    } else {
      std::set<char> ban;

      for (size_t k = 𝝅[i - 1]; k > 0; k = 𝝅[k - 1]) {
        ban.insert(str[k]);
      }

      for (char c = 'b'; c <= 'z'; ++c) {
        if (!ban.contains(c)) {
          str += c;
          break;
        }
      }
    }
  }

  return str;
}

/**
 * @brief  Constracts minimal lexicographically string from the given z-function.
 *         Time: Θ(n)
 */
std::string ZToS(const std::vector<size_t>& z) {
  // std::string str;
  // str.reserve(z.size());

  // size_t len = 0;  // prefix length
  // size_t c_i = 0;  // char index
  // size_t j = 0;    // index for prefix coping

  // auto letter = [](size_t i) -> char { return 'a' + i; };

  // str += 'a';
  // ++c_i;

  // for (size_t i = 1; i < z.size(); ++i) {
  //   if (z[i] == 0 && len == 0) {
  //     str += letter(c_i);
  //     ++c_i;
  //   }

  //   if (z[i] > len) {
  //     len = z[i];
  //     j = 0;
  //   }

  //   if (len > 0) {
  //     str += str[j];
  //     ++j;
  //     --len;
  //   }
  // }

  return PiToS(ZToPi(z));
}

}  // namespace my

#endif