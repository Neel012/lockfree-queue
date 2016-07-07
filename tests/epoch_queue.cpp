#include <epoch_queue.hpp>
#include <tests/queue_tests.hpp>
#include <tests/catch.hpp>

namespace lockfree {

TEST_CASE("epoch_queue - single thread") {
  using queue_type = epoch_queue<int>;
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
  SECTION("10 000 elements") {
    queue_type q;
    test_queue_basic(q, 10000);
  }
  SECTION("100 000 elements") {
    queue_type q;
    test_queue_basic(q, 100'000);
  }
  SECTION("1 000 000 elements") {
    queue_type q;
    test_queue_basic(q, 1'000'000);
  }
}

TEST_CASE("epoch_queue - only enqueue") {
  using queue_type = epoch_queue<int>;
  queue_type q;
  SECTION("3 elements") {
    test_only_enqueue(q, 3);
  }
  SECTION("100 elements") {
    test_only_enqueue(q, 100);
  }
  SECTION("1000 elements") {
    test_only_enqueue(q, 1000);
  }
}

TEST_CASE("ms_queue - basic threaded") {
  using queue_type = epoch_queue<int>;
  SECTION("2 threads") {
    queue_type q;
    test_queue_basic2(q, 2);
  }
  SECTION("3 threads") {
    queue_type q;
    test_queue_basic2(q, 3);
  }
  SECTION("4 threads") {
    queue_type q;
    test_queue_basic2(q, 4);
  }
  SECTION("6 threads") {
    queue_type q;
    test_queue_basic2(q, 6);
  }
}

TEST_CASE("ms_queue - two threads") {
  using queue_type = epoch_queue<int>;
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

TEST_CASE("epoch_queue - 3 threads") {
  using queue_type = epoch_queue<int>;
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
  SECTION("100 000 elements") {
    queue_type q;
    test_queue_t3(q, 100'000);
  }
  SECTION("1 200 000 elements") {
    queue_type q;
    test_queue_t3(q, 1'200'000);
  }
}

TEST_CASE("epoch_queue - many threads") {
  test_queue_manythreads<epoch_queue>(1, 1);
  test_queue_manythreads<epoch_queue>(2, 2);
  test_queue_manythreads<epoch_queue>(4, 4);
  test_queue_manythreads<epoch_queue>(6, 6);
  test_queue_manythreads<epoch_queue>(8, 8);
  test_queue_manythreads<epoch_queue>(1, 7);
  test_queue_manythreads<epoch_queue>(7, 1);
}

} // namespace lockfree
