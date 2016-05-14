#include <atomic>

template <typename T>
struct atomic_ptr {
  using pointer_type = T*;

  atomic_ptr() = default;

  atomic_ptr(pointer_type pointer) : pointer_{pointer} {}

  ~atomic_ptr() {
    reset();
  }

  void reset() {
    if (pointer_ != nullptr) {
      delete pointer_;
      pointer_ = nullptr;
    }
  }

  pointer_type load(
      std::memory_order order = std::memory_order_seq_cst) noexcept
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
      pointer_type& expected, pointer_type desired,
      std::memory_order order = std::memory_order_seq_cst)
  {
    return pointer_.compare_exchange_weak(expected, desired, order);
  }

private:
  std::atomic<pointer_type> pointer_{nullptr};
};
