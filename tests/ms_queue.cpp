#include "../ms_queue.hpp"
#include "catch.hpp"
#include "queue_tests.hpp"

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

}
