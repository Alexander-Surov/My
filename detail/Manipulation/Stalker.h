#ifndef STALKER
#define STALKER

#include <memory>

namespace my {

  /// Relation scheme:
  ///                                 ┏╸Owner╺━━━━━━━━┳━━━━━┓
  ///                            ┏━━► ┃ ... ┃ Stalker ┃ ... ┃
  ///                            ┃    ┗━━━━━┻━━━━┳━━━━┻━━━━━┛
  ///  ┏━━━━━━━━━━━┓ ╺╺╺╺━►  ┏━━━┻━━┓            ┃
  ///  ┃ accessors ┣━━━━━━━► ┃ Dock ┃ ◄━━━━━━━━━━┛
  ///  ┗━━━━━━━━━━━┛ ╺╺╺╺━►  ┗━━━━━━┛

  /**
   * @brief `Stalker` allows to track current address of `Owner` data
   *
   * @details `Stalker` shares class `accessor` which provides access to the run-time `Owner` via `ref()` or `ptr()`
   *
   * @note `Owner` must have only one field of type `Stalker`, which must be named `stalker_`
   *
   * @tparam  Owner  type of class that owns `Stalker`, i.e. the one that needs to be tracked
   */
  template <typename Owner>
  class Stalker {

    struct Dock;
    class Base;
    class Accessor;
    class ConstAccessor;


   public:
    using value_type      = Owner;

    using reference       = value_type&;
    using const_reference = const value_type&;

    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    using accessor        = Accessor;
    using const_accessor  = ConstAccessor;


   private:
    struct Dock {
      pointer ptr;

     public:
      explicit Dock(const pointer& p) noexcept : ptr(p) {
      }
      ~Dock() = default;
    };


    class Base {
      friend Stalker;

     protected:
      Dock* dock_ptr_;

     protected:
      explicit Base(Dock* const& p) noexcept : dock_ptr_(p) {
      }

     public:
      Base() noexcept : dock_ptr_(nullptr) {
      }
      ~Base() = default;
    };


    class Accessor : public Base {
      friend ConstAccessor;

     public:
      using Base::Base;

      Accessor(const ConstAccessor&) = delete;
      Accessor(ConstAccessor&&) = delete;

      Accessor& operator=(const ConstAccessor&) = delete;
      Accessor& operator=(ConstAccessor&&) = delete;

     public:
      reference ref() const noexcept {
        return *ptr();
      }
      pointer ptr() const noexcept {
        return Base::dock_ptr_->ptr;
      }
      operator bool() const noexcept {
        return ptr() != nullptr;
      }
    };


    class ConstAccessor : public Base {
      friend Accessor;

     public:
      using Base::Base;

      ConstAccessor(const Accessor& other) noexcept : Base(other.Accessor::dock_ptr_) {
      }
      ConstAccessor(Accessor&& other) noexcept : Base(other.Accessor::dock_ptr_) {
      }

      ConstAccessor& operator=(const Accessor& other) noexcept {
        Base::dock_ptr_ = other.Accessor::dock_ptr_;
        return *this;
      }
      ConstAccessor& operator=(Accessor&& other) noexcept {
        Base::dock_ptr_ = other.Accessor::dock_ptr_;
        return *this;
      }

     public:
      const_reference ref() const noexcept {
        return *ptr();
      }
      const_pointer ptr() const noexcept {
        return Base::dock_ptr_->ptr;
      }
      operator bool() const noexcept {
        return ptr() != nullptr;
      }
    };


   private:
    std::unique_ptr<Dock> dock_;
    static constexpr ptrdiff_t shift_ = offsetof(value_type, stalker_);


   public:
    Stalker() : dock_(std::make_unique<Dock>(owner_ptr())) {
    }

    Stalker(const Stalker& other) : dock_(std::make_unique<Dock>(owner_ptr())) {
    }
    Stalker(Stalker&& other) noexcept : dock_(std::move(other.dock_)) {
      dock_->ptr = owner_ptr();
    }

    Stalker& operator=(const Stalker& other) noexcept {
      return *this;
    }
    Stalker& operator=(Stalker&& other) noexcept {
      if (this != &other) {
        // std::swap(dock_, other.dock_);
        dock_ = std::move(other.dock_);
        dock_->ptr = owner_ptr();
      }
      return *this;
    }

    ~Stalker() = default;


   private:
    pointer owner_ptr() const noexcept {
      return reinterpret_cast<pointer>(reinterpret_cast<char*>(const_cast<Stalker*>(this)) - shift_);
    }


   public:
    accessor get() noexcept {
      return accessor(dock_.get());
    }
    const_accessor get() const noexcept {
      return const_accessor(dock_.get());
    }
  };

}

#endif