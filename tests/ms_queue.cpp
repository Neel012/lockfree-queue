#include <ms_queue.hpp>
#include <tests/queue_tests.hpp>
#include <tests/catch.hpp>

namespace lockfree {

TEST_CASE("ms_queue - single thread") {
  using queue_type = ms_queue<int>;
  SECTION("3 elements") {
    queue_type q;
    test_queue_basic(q, 3);
  }
  SECTION("100 elements") {
    queue_type q;
    test_queue_basic(q, 100);
  }
  SECTION("1000 elements") {
    queue_type q;
    test_queue_basic(q, 1000);
  }
  SECTION(">100 000 elements") {
    queue_type q;
    test_queue_basic(q, 1000000);
  }
}

TEST_CASE("ms_queue - only enqueue") {
  using queue_type = ms_queue<int>;
  SECTION("3 elements") {
    queue_type q;
    test_only_enqueue(q, 3);
  }
  SECTION("100 elements") {
    queue_type q;
    test_only_enqueue(q, 100);
  }
  SECTION("1000 elements") {
    queue_type q;
    test_only_enqueue(q, 1000);
  }
}

TEST_CASE("ms_queue - basic async") {
  using queue_type = ms_queue<int>;
  SECTION("3 elements") {
    queue_type q;
    test_queue_basic2(q, 3);
  }
  SECTION("100 elements") {
    queue_type q;
    test_queue_basic2(q, 100);
  }
  SECTION("1000 elements") {
    queue_type q;
    test_queue_basic2(q, 1000);
  }
}

TEST_CASE("ms_queue - two threads") {
  using queue_type = ms_queue<int>;
  SECTION("3 elements") {
    queue_type q;
    test_queue_t2(q, 3);
  }
  SECTION("100 elements") {
    queue_type q;
    test_queue_t2(q, 100);
  }
  SECTION("1000 elements") {
    queue_type q;
    test_queue_t2(q, 1000);
  }
}

TEST_CASE("ms_queue - 3 threads") {
  using queue_type = ms_queue<int>;
  SECTION("3 elements") {
    queue_type q;
    test_queue_t3(q, 3);
  }
  SECTION("100 elements") {
    queue_type q;
    test_queue_t3(q, 100);
  }
  SECTION("1000 elements") {
    queue_type q;
    test_queue_t3(q, 1000);
  }
  SECTION(">100 000 elements") {
    queue_type q;
    test_queue_t3(q, 200000);
  }
}

TEST_CASE("ms_queue - many threads") {
  test_queue_manythreads<ms_queue>(1, 1);
  test_queue_manythreads<ms_queue>(2, 2);
  test_queue_manythreads<ms_queue>(4, 4);
  test_queue_manythreads<ms_queue>(6, 6);
  test_queue_manythreads<ms_queue>(8, 8);
  test_queue_manythreads<ms_queue>(1, 7);
  test_queue_manythreads<ms_queue>(7, 1);
}

} // namespace lockfree
