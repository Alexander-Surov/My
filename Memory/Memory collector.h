#ifndef MEMORY_COLLECTOR
#define MEMORY_COLLECTOR

#include <utility>

namespace my {

/**
 * @brief   A container collecting raw memory.
 *
 * @tparam  T  Extra type to be stored within each collected memory block (defaults to `void`)
 */
template <typename Ext = void>
class MemoryCollector {
 public:
  using ext_type = Ext;


 private:
  static constexpr bool is_ext_void = std::is_void_v<std::remove_cv_t<ext_type>>;

  template <typename... Args>
  static constexpr bool is_ext_constructible = std::is_constructible_v<ext_type, Args...> ||
                                               (is_ext_void && sizeof...(Args) == 0);


 private:
  struct Frame {
    [[no_unique_address]] std::conditional_t<is_ext_void, std::false_type, ext_type> ext;
    Frame* next;

    template <typename... Args>
    requires(is_ext_constructible<Args...>)
    Frame(Frame* p, Args&&... args) : ext(std::forward<Args>(args)...), next(p) {
    }
  };


 private:
  Frame* ptr_ = nullptr;
  Frame* front_ = nullptr;


 public:
  /**
   * @brief Minimum required number of bytes for any memory block to be collected.
   */
  static constexpr std::size_t req_space = sizeof(Frame);


 public:
  MemoryCollector() = default;


 public:
  /**
   * @brief Puts the memory to the stack top.
   *
   * @note Memory at `p` must be at least `mem_space` bytes.
   *
   * @param  p     Pointer to the memory
   * @param  args  Arguments for construction of `ext_type` at `p` (if `ext_type` is not `void`)
   */
  template <typename... Args>
  requires(is_ext_constructible<Args...>)
  void push(void* p, Args&&... args) {
    if (empty()) {
      front_ = static_cast<Frame*>(p);
    }

    ptr_ = new (p) Frame(ptr_, std::forward<Args>(args)...);
  }

  /**
   * @brief Returns pointer to the memory from the stack top and pops it from the storage.
   *
   * @note The asociated `ext_type` (if `ext_type` is not `void`) at the stack top is destoyed: call `top_ext()` ahead.
   */
  void* pop() {
    auto p = ptr_;
    ptr_ = ptr_->next;
    p->~Frame();

    if (empty()) {
      front_ = nullptr;
    }

    return static_cast<void*>(p);
  }

  /**
   * @brief Returns the `ext_type` reference at the stack top.
   *
   * @note This method can only be called if `ext_type` is not `void`.
   */
  decltype(auto) top_ext() requires(not is_ext_void) {
    return ptr_->ext;
  }

  /**
   * @brief  Retrieves and collects memory blocks from `other`.
   *
   * @param  other  `MemoryCollector` to be absorbed
   */
  void absorb(MemoryCollector& other) noexcept {
    if (!empty()) {
      front_->next = other.ptr_;
    }
    if (!other.empty()) {
      front_ = other.front_;
    }
    other.front_ = other.ptr_ = nullptr;
  }

  /**
   * @brief Indicates whether `MemoryCollector` has any memory or not.
   */
  bool empty() const noexcept {
    return ptr_ == nullptr;
  }
};

}  // namespace my

#endif