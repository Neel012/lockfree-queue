#include <epoch_queue.hpp>
#include <tests/queue_tests_run.hpp>
#include <tests/catch.hpp>

namespace lockfree {

template <typename T>
using Q = epoch_queue<T>;

TEST_CASE("epoch_queue - single thread") {
  run_test_queue_basic<Q>();
}

TEST_CASE("epoch_queue - basic threaded") {
  run_test_queue_basic2<Q>();
}

TEST_CASE("epoch_queue - two threads") {
  run_test_queue_t2<Q>();
}

TEST_CASE("epoch_queue - 3 threads") {
  run_test_queue_t3<Q>();
}

TEST_CASE("epoch_queue - many threads") {
  run_test_manythreads<Q>();
}

} // namespace lockfree
