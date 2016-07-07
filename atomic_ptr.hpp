#pragma once

#include <atomic>
#include <utility>

namespace lockfree {

template <typename T>
struct atomic_ptr {
  using pointer_type = T*;

  atomic_ptr() = default;

  atomic_ptr(const atomic_ptr&) = delete;

  atomic_ptr(atomic_ptr&& rhs) noexcept : pointer_{rhs.release()} { }

  explicit atomic_ptr(pointer_type pointer) noexcept : pointer_{pointer}
  { }

  ~atomic_ptr() noexcept {
    reset();
  }

  atomic_ptr& operator=(const atomic_ptr&) = delete;

  atomic_ptr& operator=(atomic_ptr&& rhs) noexcept {
    reset(rhs.release());
    return *this;
  }

  pointer_type operator->() const noexcept {
    return pointer_.load();
  }

  void reset(std::memory_order order = std::memory_order_seq_cst) noexcept {
    pointer_type p = release(order);
    if (p != nullptr) {
      delete p;
    }
  }

  pointer_type reset(
      pointer_type p,
      std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    pointer_type old = pointer_.exchange(p, order);
    if (old != nullptr) {
      delete old;
    }
  }

  pointer_type release(
      std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    return pointer_.exchange(nullptr, order);
  }

  pointer_type load(
      std::memory_order order = std::memory_order_seq_cst) const noexcept
  {
    return pointer_.load(order);
  }

  void store(
      pointer_type& desired,
      std::memory_order order = std::memory_order_seq_cst) noexcept
  {
    pointer_.store(desired, order);
  }

  bool compare_exchange_weak(
      pointer_type& expected,
      pointer_type desired,
      std::memory_order order = std::memory_order_seq_cst) noexcept
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

} // namespace lockfree
