#ifndef FREELIST_ALLOCATOR
#define FREELIST_ALLOCATOR

#include <memory_resource>
#include "Memory collector.h"

namespace my {

/**
 * @brief   A data structure used in a scheme for dynamic memory allocation. It operates by connecting
 *          unallocated regions of memory together in a linked list, using the first word of each
 *          unallocated region as a pointer to the next. It is most suitable for allocating from a memory pool,
 *          where all objects have the same size.
 *
 * @tparam  T  Type of data
 */
template <typename T>
class FreelistAllocator {

  static_assert(!std::is_void_v<std::remove_cv_t<T>>, "value_type must be non-void");


 public:
  using value_type = T;
  using pointer = T*;

  template <typename U>
  struct rebind {
    using other = FreelistAllocator<U>;
  };


 private:
  struct Group {
    size_t size;
  };


 private:
  std::pmr::memory_resource* resource_;
  MemoryCollector<> free_;
  MemoryCollector<Group> groups_;


 public:
  // Default //

  FreelistAllocator() noexcept : FreelistAllocator(std::pmr::get_default_resource()) {
  }

  FreelistAllocator(std::pmr::memory_resource* r) noexcept : resource_(r) {
  }


  // Copy & assignment //

  FreelistAllocator(const FreelistAllocator& other) noexcept : FreelistAllocator(other.resource()) {
  }
  FreelistAllocator(FreelistAllocator&& other) noexcept {
    std::swap(resource_, other.resource_);
    std::swap(free_, other.free_);
    std::swap(groups_, other.groups_);
  }

  template <typename U>
  FreelistAllocator(const FreelistAllocator<U>& alloc) noexcept : FreelistAllocator(alloc.resource()) {
  }

  FreelistAllocator& operator=(const FreelistAllocator& other) {
    resource_ = other.resource();
    return *this;
  }
  FreelistAllocator& operator=(FreelistAllocator&& other) noexcept {
    std::swap(resource_, other.resource_);
    std::swap(free_, other.free_);
    std::swap(groups_, other.groups_);
    return *this;
  }

  template <typename U>
  FreelistAllocator& operator=(const FreelistAllocator<U>& alloc) {
    resource_ = alloc.resource();
    return *this;
  }


  // Destructor //

  ~FreelistAllocator() {
    while (!groups_.empty()) {
      size_t n = groups_.top_ext().size;
      pointer p = static_cast<pointer>(groups_.pop());
      resource_->deallocate(p - n, group_size_in_bytes(n), alignof(value_type));
    }
  }


 private:
 /**
  * ┏━━━┳━━━━━┳━━━┳━━━━━━━┓
  * ┃ T ┃ ... ┃ T ┃ GROUP ┃
  * ┗━━━┻━━━━━┻━━━┻━━━━━━━┛
  *
  * @param  n  Number of elements in the group
  *
  * @return  Group size in bytes
  */
  static size_t group_size_in_bytes(size_t n) noexcept {
    return n * sizeof(value_type) + decltype(groups_)::req_space;
  }


 public:
  /**
   * @brief n = 1: Reuses previously deallocated `value_type` if the cache is non-empty or allocates from the resource
   *        n > 1: Allocates a contiguous pack of `n` `value_types`
   *
   * @param  n  Number of `value_types` to allocate
   *
   * @return  Pointer to the (first) `value_type`
   */
  [[nodiscard]]
  pointer allocate(size_t n) {
    if (n == 1 && !free_.empty()) {
      return static_cast<pointer>(free_.pop());
    }

    pointer p = static_cast<pointer>(resource_->allocate(group_size_in_bytes(n), alignof(value_type)));
    groups_.push(p + n, n);

    return p;
  }

  /**
   * @brief Accumulates deallocated `value_type(s)`
   *
   * @param  p  Pointer to the `value_type(s)` to deallocate
   * @param  n  Number of `value_types`
   */
  void deallocate(pointer p, size_t n) noexcept {
    for (; n > 0; ++p, --n) {
      free_.push(p);
    }
  }

  /**
   * @brief Reserves free `value_types` for further single allocations
   *
   * @param  n  Number of `value_types` to reserve
   */
  void reserve(size_t n) {
    deallocate(allocate(n), n);
  }

  /***/
  void absorb(FreelistAllocator& other) {
    if (resource() != other.resource()) {
      throw std::runtime_error("Allocators have different sources");
    }

    groups_.absorb(other.groups_);
    free_.absorb(other.free_);
  }

  /**
   * @return  Pointer to the resource
   */
  std::pmr::memory_resource* resource() const noexcept {
    return resource_;
  }
};


template <typename T, typename U>
bool operator==(const FreelistAllocator<T>& a, const FreelistAllocator<U>& b) noexcept {
  return *a.resource() == *b.resource();
}

}  // namespace my

#endif