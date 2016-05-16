#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <utility>

// Type erased smart pointer
struct any_ptr {
  using pointer_type = void*;
  using deleter_type = void(pointer_type);

  template <typename E>
  struct default_delete {
    void operator()(void* ptr) {
      //delete static_cast<E*>(ptr);
      E* e = static_cast<E*>(ptr);
      e->next.release();
      delete e;
    }
  };

  any_ptr() = default;

  any_ptr(any_ptr&& rhs) noexcept
      : pointer_{std::move(rhs.pointer_)}, deleter_{std::move(rhs.deleter_)}
  {
    rhs.pointer_ = nullptr;
  }

  template <typename E>
  any_ptr(
      E* pointer,
      std::function<deleter_type>&& deleter = default_delete<E>{}) noexcept
      : pointer_{pointer}, deleter_{std::move(deleter)}
  { }

  ~any_ptr() noexcept {
    reset();
  }

  void reset() noexcept {
    if (pointer_ != nullptr) {
      deleter_(pointer_);
      pointer_ = nullptr;
    }
  }

  pointer_type get() const {
    return pointer_;
  }

  pointer_type operator->() const {
    return get();
  }

  bool operator==(const any_ptr& rhs) {
    return get() == rhs.get();
  }

  bool operator==(std::nullptr_t) {
    return get() == nullptr;
  }

private:
  pointer_type pointer_{nullptr};
  std::function<deleter_type> deleter_;
};

