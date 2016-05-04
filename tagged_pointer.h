#include <atomic>
#include <cstdint>

namespace lockfree {

// TODO: determine which bits get used in address
const std::uint64_t ptr_mask = 0xffffffffffffUL;
const std::uint64_t counter_mask;
const std::uint16_t counter_tag[4];

union pointer_value {
  uint64_t ptr;
  uint16_t counter_tag[4];
};

template <typename T>
struct tagged_pointer {
  T* ptr() {
    return ptr_mask & value_;
  }

  uint16_t count() {
    pointer_value x;
    x.ptr = value_;
    return x.counter_tag[3];
  }

private:
  uint64_t value_;
};

}
