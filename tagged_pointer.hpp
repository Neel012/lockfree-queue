#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace lockfree {

template <typename T>
struct tagged_pointer {
  tagged_pointer() = default;

  tagged_pointer(T* ptr, uint16_t count = 0) noexcept {
    value_.ptr = uint64_t(ptr);
    value_.counter_tag[3] = count;
  }

  T* ptr() const {
    return (T*)(ptr_mask & value_.ptr);
  }

  uint16_t count() const {
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

  static const std::uint64_t ptr_mask = 0xffffffffffffUL;

  pointer_value value_;
};

} // namespace lockfree

