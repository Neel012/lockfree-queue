#pragma once

#include <tests/catch.hpp>
#include <tests/queue_tests.hpp>

namespace lockfree {

template <template <typename> class Q>
void run_test_queue_basic() {
  SECTION("3 elements") {
    test_queue_basic<Q>(3);
  }
  SECTION("100 elements") {
    test_queue_basic<Q>(100);
  }
  SECTION("1000 elements") {
    test_queue_basic<Q>(1000);
  }
  SECTION(">100 000 elements") {
    test_queue_basic<Q>(1000000);
  }
}

template <template <typename> class Q>
void run_test_queue_basic2() {
  SECTION("2 threads") {
    test_queue_basic2<Q>(2);
  }
  SECTION("3 threads") {
    test_queue_basic2<Q>(3);
  }
  SECTION("4 threads") {
    test_queue_basic2<Q>(4);
  }
  SECTION("6 threads") {
    test_queue_basic2<Q>(6);
  }
}

template <template <typename> class Q>
void run_test_queue_t2() {
  SECTION("3 elements") {
    test_queue_t2<Q>(3);
  }
  SECTION("100 elements") {
    test_queue_t2<Q>(100);
  }
  SECTION("1000 elements") {
    test_queue_t2<Q>(1000);
  }
}

template <template <typename> class Q>
void run_test_queue_t3() {
  SECTION("3 elements") {
    test_queue_t3<Q>(3);
  }
  SECTION("100 elements") {
    test_queue_t3<Q>(100);
  }
  SECTION("1000 elements") {
    test_queue_t3<Q>(1000);
  }
  SECTION(">100 000 elements") {
    test_queue_t3<Q>(200000);
  }
}

template <template <typename> class Q>
void run_test_manythreads() {
  std::size_t messages = 50;
  SECTION("50 messages") {
    messages = 50;
    test_queue_manythreads<Q>(1, 1, messages);
    test_queue_manythreads<Q>(2, 2, messages);
    test_queue_manythreads<Q>(4, 4, messages);
    test_queue_manythreads<Q>(6, 6, messages);
    test_queue_manythreads<Q>(8, 8, messages);
    test_queue_manythreads<Q>(1, 7, messages);
    test_queue_manythreads<Q>(7, 1, messages);
  }
  SECTION("100 messages") {
    messages = 100;
    test_queue_manythreads<Q>(1, 1, messages);
    test_queue_manythreads<Q>(2, 2, messages);
    test_queue_manythreads<Q>(4, 4, messages);
    test_queue_manythreads<Q>(6, 6, messages);
    test_queue_manythreads<Q>(8, 8, messages);
    test_queue_manythreads<Q>(1, 7, messages);
    test_queue_manythreads<Q>(7, 1, messages);
  }
  SECTION("150 messages") {
    messages = 150;
    test_queue_manythreads<Q>(1, 1, messages);
    test_queue_manythreads<Q>(2, 2, messages);
    test_queue_manythreads<Q>(4, 4, messages);
    test_queue_manythreads<Q>(6, 6, messages);
    test_queue_manythreads<Q>(8, 8, messages);
    test_queue_manythreads<Q>(1, 7, messages);
    test_queue_manythreads<Q>(7, 1, messages);
  }
  SECTION("300 messages") {
    messages = 300;
    test_queue_manythreads<Q>(1, 1, messages);
    test_queue_manythreads<Q>(2, 2, messages);
    test_queue_manythreads<Q>(4, 4, messages);
    test_queue_manythreads<Q>(6, 6, messages);
    test_queue_manythreads<Q>(8, 8, messages);
    test_queue_manythreads<Q>(1, 7, messages);
    test_queue_manythreads<Q>(7, 1, messages);
  }
  SECTION("10 000 messages") {
    messages = 10'000;
    test_queue_manythreads<Q>(1, 1, messages);
    test_queue_manythreads<Q>(2, 2, messages);
    test_queue_manythreads<Q>(4, 4, messages);
    test_queue_manythreads<Q>(6, 6, messages);
    test_queue_manythreads<Q>(8, 8, messages);
    test_queue_manythreads<Q>(1, 7, messages);
    test_queue_manythreads<Q>(7, 1, messages);
  }
}

} // namespace lockfree

