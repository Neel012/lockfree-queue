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

}
