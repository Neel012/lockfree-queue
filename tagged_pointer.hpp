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
    return (T*)(ptr_mask & value_.ptr);
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

TEST_CASE("tagged_pointer - on stack") {
  int i = 42;

  SECTION("basic test") {
    tagged_pointer<int> tagptr(&i, 5);
    REQUIRE(tagptr.count() == 5);
    REQUIRE(tagptr.ptr() == &i);
    REQUIRE(*tagptr.ptr() == i);
  }
  SECTION("size test") {
    tagged_pointer<int> a(&i, 5);
    REQUIRE(sizeof(a) == sizeof(&i));
  }
  SECTION("operator==") {
    tagged_pointer<int> a(&i, 5);
    tagged_pointer<int> b(&i, 5);
    REQUIRE(a == b);
  }
}

TEST_CASE("tagged_pointer - on heap") {
  int* i = new int{42};

  SECTION("basic test") {
    int* i = new int{42};
    tagged_pointer<int> tagptr(i, 5);
    REQUIRE(tagptr.count() == 5);
    REQUIRE(tagptr.ptr() == i);
    REQUIRE(*tagptr.ptr() == *i);
  }
  SECTION("size test") {
    int* i = new int{42};
    tagged_pointer<int> a(i, 5);
    REQUIRE(sizeof(a) == sizeof(i));
  }
  SECTION("operator==") {
    tagged_pointer<int> a(i, 5);
    tagged_pointer<int> b(i, 5);
    REQUIRE(a == b);
  }
  delete i;
}

}