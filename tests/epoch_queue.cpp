#define CATCH_CONFIG_MAIN
#include <future>
#include <thread>
#include "catch.hpp"
#include "../epoch_queue.hpp"

namespace lockfree {

TEST_CASE("epoch_queue - single thread") {
  SECTION("3 elements") {
    epoch_queue<int> q;
    REQUIRE(q.empty());
    q.enqueue(1);
    REQUIRE(!q.empty());
    q.enqueue(2);
    REQUIRE(!q.empty());
    q.enqueue(3);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(q.empty());
  }

  SECTION("3 elements interleaved") {
    epoch_queue<int> q;
    REQUIRE(q.empty());
    q.enqueue(1);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(q.empty());
    q.enqueue(2);
    REQUIRE(!q.empty());
    q.enqueue(3);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(q.empty());
  }
}

void maybe_recieve(epoch_queue<int>& q, unsigned count) {
  unsigned i = 1;
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

TEST_CASE("epoch_queue - two threads") {
  std::thread t1, t2;
  SECTION("3 elements") {
    epoch_queue<int> q;
    t1 = std::thread([&] {
      q.enqueue(1);
      q.enqueue(2);
      q.enqueue(3);
    });
    t2 = std::thread([&] {
        maybe_recieve(q, 3);
    });
    t1.join();
    t2.join();
  }
}

}
