#pragma once

#include <atomic>
#include <utility>

template <typename T>
struct atomic_ptr {
  using pointer_type = T*;

  atomic_ptr() = default;

  atomic_ptr(pointer_type pointer) : pointer_{pointer} {}

  ~atomic_ptr() {
    reset();
  }

  bool operator==(atomic_ptr& rhs) {
    return load() == rhs.load();
  }

  bool operator==(std::nullptr_t) {
    return load() == nullptr;
  }

  pointer_type operator->() const {
    return pointer_.load();
  }

  void reset() {
    if (pointer_ != nullptr) {
      delete pointer_;
      pointer_ = nullptr;
    }
  }

  void release() {
    pointer_.store(nullptr);
  }

  pointer_type load(
      std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return pointer_.load(order);
  }

  void store(
      pointer_type& desired,
      std::memory_order order = std::memory_order_seq_cst)
  {
    pointer_.store(desired, order);
  }

  bool compare_exchange_weak(
      pointer_type expected,
      pointer_type desired,
      std::memory_order order = std::memory_order_seq_cst) volatile
  {
    return pointer_.compare_exchange_weak(expected, desired, order);
  }

private:
  std::atomic<pointer_type> pointer_{nullptr};
};

template <typename T, typename... Args>
atomic_ptr<T> make_atomic_ptr(Args&&... args) {
  return atomic_ptr<T>(new T{std::forward<Args>(args)...});
}

