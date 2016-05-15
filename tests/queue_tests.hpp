#pragma once

#include "catch.hpp"
#include <future>
#include <thread>

namespace lockfree {

template <template <typename> class Q>
void test_queue_basic(Q<int>& q, size_t count) {
  REQUIRE(q.empty());
  for (size_t i{0}; i < count; i++) {
    q.enqueue(i + 1);
  }
  REQUIRE(!q.empty());
  for (size_t i{0}; i < count; i++) {
    REQUIRE(*q.dequeue() == i + 1);
  }
  REQUIRE(q.empty());
}

template <typename Q>
void maybe_recieve(Q& q, size_t count) {
  int i = 1;
  while (i <= count) {
    //REQUIRE(!q.empty());
    auto pop = q.dequeue();
    if (pop) {
      REQUIRE(*pop == i);
      i++;
    }
  }
  REQUIRE(q.empty());
}

template <template <typename> class Q>
void test_queue_t2(Q<int>& q, size_t n) {
  auto t1 = std::thread([&] {
    for (size_t i{0}; i < n; i++) {
      q.enqueue(i + 1);
    }
  });
  auto t2 = std::thread([&] { maybe_recieve(q, n); });
  t1.join();
  t2.join();
}

} // namespace lockfree
