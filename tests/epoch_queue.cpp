#include "../epoch_queue.hpp"
#include "queue_tests.hpp"
#include "catch.hpp"

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

TEST_CASE("ms_queue - basic async") {
  using queue_type = epoch_queue<int>;
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
  SECTION(">100 000 elements") {
    queue_type q;
    test_queue_t3(q, 120000);
  }
}

}
