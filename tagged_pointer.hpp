#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>

namespace lockfree {

template <typename T>
struct tagged_pointer {
  tagged_pointer() = default;

  tagged_pointer(T* ptr, uint16_t count = 0) noexcept {
    static_assert(sizeof(ptr) == sizeof(uint64_t), "Pointer needs to be of size 64 bits");
    assert(is_little_endian());
    value_.ptr = uint64_t(ptr);
    value_.counter_tag[3] = count;
    assert(ptr == this->ptr());
  }

  T* ptr() const noexcept {
    return reinterpret_cast<T*>(ptr_mask & value_.ptr);
  }

  uint16_t count() const noexcept {
    return value_.counter_tag[3];
  }

  bool operator==(const tagged_pointer& rhs) const noexcept {
    return value_.ptr == rhs.value_.ptr;
  }

  bool operator!=(const tagged_pointer& rhs) const noexcept {
    return !(*this == rhs);
  }

  bool operator==(std::nullptr_t) const noexcept {
    return ptr() == nullptr;
  }

  bool operator!=(std::nullptr_t) const noexcept {
    return !(*this == nullptr);
  }

private:
  union pointer_value {
    uint64_t ptr;
    uint16_t counter_tag[4];
  };

  static bool is_little_endian() {
    pointer_value e;
    e.ptr = {0x0002000400060008};
    return 0x0002 == e.counter_tag[3];
  }

  static const std::uint64_t ptr_mask = 0xffffffffffffUL;

  pointer_value value_;
};

} // namespace lockfree

