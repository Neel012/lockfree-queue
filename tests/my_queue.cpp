#include <my_queue.hpp>
#include <tests/queue_tests_run.hpp>
#include <tests/catch.hpp>

namespace lockfree {

template <typename T>
using Q = my_queue<T>;

TEST_CASE("my_queue - single thread") {
  run_test_queue_basic<Q>();
}

TEST_CASE("my_queue - basic threaded") {
  run_test_queue_basic2<Q>();
}

TEST_CASE("my_queue - two threads") {
  run_test_queue_t2<Q>();
}

TEST_CASE("my_queue - 3 threads") {
  run_test_queue_t3<Q>();
}

TEST_CASE("my_queue - dequeue order") {
  run_test_dequeue_order<Q>();
}

TEST_CASE("my_queue - many threads") {
  run_test_manythreads<Q>();
}

} // namespace lockfree
