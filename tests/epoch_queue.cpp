#define CATCH_CONFIG_MAIN
#include "../epoch_queue.hpp"
#include "catch.hpp"

namespace lockfree {

TEST_CASE("epoch_queue - single thread") {
  SECTION("3 elements") {
    epoch_queue<int> q;
    REQUIRE(q.empty());
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(q.empty());
  }
}

}
