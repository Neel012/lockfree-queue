#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../ms_queue.h"
#include "../epoch_queue.h"



TEST_CASE("Basic tests - single thread") {
    ms_queue<int> q;
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(*q.dequeue() == 3);
}