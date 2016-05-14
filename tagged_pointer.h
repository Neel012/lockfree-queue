#pragma once

#include <atomic>
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
    return static_cast<T*>(ptr_mask & value_.ptr);
  }

  uint16_t count() const {
    return value_.counter_tag[3];
  }

  bool operator==(const tagged_pointer& rhs) {
    return count() == rhs.count() && ptr() == rhs.ptr();
  }

private:
  union pointer_value {
    uint64_t ptr;
    uint16_t counter_tag[4];
  };

  static const std::uint64_t ptr_mask = 0xffffffffffffUL;

  pointer_value value_;
};


}
