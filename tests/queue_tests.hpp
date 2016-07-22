#pragma once

#include <algorithm>
#include <future>
#include <thread>

#include <tests/catch.hpp>

namespace lockfree {

template <template <typename> class Q>
void test_queue_basic(std::size_t count) {
  Q<std::size_t> q;
  REQUIRE(q.empty());
  for (std::size_t i = 0; i < count; ++i) {
    q.enqueue(i);
  }
  REQUIRE(!q.empty());
  for (std::size_t i = 0; i < count; ++i) {
    REQUIRE(*q.dequeue() == i);
  }
  REQUIRE(q.empty());
}

template <typename Q>
void maybe_recieve(Q& q, std::size_t count) {
  for (std::size_t i = 1; i <= count;) {
    auto pop = q.dequeue();
    if (pop) {
      REQUIRE(*pop == i);
      ++i;
    }
  }
}

template <typename Q>
void maybe_recieve_anything(Q& q, std::size_t count) {
  for (std::size_t i = 1; i <= count;) {
    if (q.dequeue()) {
      ++i;
    }
  }
}

template <template <typename> class Q>
void test_queue_basic2(std::size_t count) {
  Q<int> q;
  REQUIRE(q.empty());
  std::vector<std::thread> threads(count);
  for (auto& t : threads) {
    t = std::thread([&] {
      q.enqueue(1);
      maybe_recieve_anything(q, 1);
    });
  }
  for (auto& t : threads) {
    t.join();
  }
  REQUIRE(q.empty());
}

template <template <typename> class Q>
void test_queue_t2(std::size_t n) {
  Q<std::size_t> q;
  auto t = std::thread([&] {
    for (std::size_t i = 0; i < n; ++i) {
      q.enqueue(i + 1);
    }
  });
  maybe_recieve(q, n);
  t.join();
  REQUIRE(q.empty());
}

template <template <typename> class Q>
void test_queue_t3(std::size_t n) {
  Q<int> q;
  auto t1 = std::thread([&] { maybe_recieve_anything(q, n); });
  auto t2 = std::thread([&] {
    for (std::size_t i = 0; i < n/2; ++i) {
      q.enqueue(i + 1);
    }
  });
  for (std::size_t i = 0; i < (n / 2) + (n % 2); ++i) {
    q.enqueue(i + 1);
  }
  t1.join();
  t2.join();
  REQUIRE(q.empty());
}

template <typename T>
using Vec2D = std::vector<std::vector<T>>;

struct producer_value {
  producer_value() = default;
  producer_value(unsigned p, int v) : producer{p}, value{v} {}
  unsigned producer;
  int value;
  char padding[256];
};

template <typename Q>
void produce(Q& q, unsigned producer_num, std::vector<int>& testset) {
  for (auto& v : testset) {
    q.enqueue(producer_value{producer_num, v});
  }
}

bool check_order(std::vector<producer_value>& received, std::size_t chunk, unsigned producers_count) {
  std::vector<int> producer_values(producers_count, 0);
  std::size_t i = 0;
  for (auto& pv : received) {
    if (producer_values[pv.producer] >= pv.value) {
      return false;
    }
    producer_values[pv.producer] = pv.value;
    ++i;
  }
  return chunk == i;
}

template <typename Q>
bool consume(Q& q, std::size_t chunk, unsigned producers_count) {
  std::vector<producer_value> received(chunk);
  for (std::size_t i = 0; i < chunk;) {
    auto r = q.dequeue();
    if (r) {
      received[i] = *r;
      ++i;
    }
  }
  return check_order(received, chunk, producers_count);
}

Vec2D<int> gen_seqset(unsigned producers_count, std::size_t chunk) {
  Vec2D<int> testset(producers_count, std::vector<int>(chunk));
  unsigned i{1};
  for (auto& s : testset) {
    std::generate(s.begin(), s.end(), [&i]{ return i++; });
  }
  return testset;
}

constexpr unsigned gcd(unsigned p, unsigned c) {
  while (c != 0) {
    unsigned tmp = p % c;
    p = c;
    c = tmp;
  }
  return p;
}

constexpr unsigned lcm(unsigned p, unsigned c) {
  unsigned d = gcd(p, c);
  return d ? (p * c) / d : 0;
}

constexpr std::size_t remainderless(std::size_t base, unsigned desired_denominator) {
  base += desired_denominator - (base % desired_denominator);
  return base;
}

// Distribute @messages among @producers_count producers, let @consumers_count
// consumers receive same amount of @messages combined.
// Assert that no message was lost and no consumer has received message in a
// wrong order.
template <template <typename> class Q>
void test_queue_manythreads(
    unsigned producers_count,
    unsigned consumers_count,
    std::size_t messages)
{
  messages = remainderless(messages, lcm(producers_count, consumers_count));
  Vec2D<int> testset = gen_seqset(producers_count, messages / producers_count);
  std::vector<std::thread> producers(producers_count);
  std::vector<std::future<bool>> valid_orders(consumers_count);
  Q<producer_value> q;
  for (unsigned i = 0; i < producers_count; ++i) {
    producers[i] = std::thread(&produce<decltype(q)>, std::ref(q), i, std::ref(testset[i]));
  }
  for (auto& f : valid_orders) {
    f = std::async(std::launch::async, [&]{ return consume(q, messages / consumers_count, producers_count); });
  }
  std::for_each(producers.begin(), producers.end(), [](auto& t){ t.join(); } );
  std::for_each(valid_orders.begin(), valid_orders.end(), [](auto& f){ REQUIRE(f.get()); });
  REQUIRE(q.empty());
}

template <template <typename> class Q>
void test_dequeue_order(unsigned consumers_count, std::size_t messages) {
  messages = remainderless(messages, consumers_count);
  Vec2D<int> testset(1, std::vector<int>(messages));
  std::vector<std::future<bool>> valid_orders(consumers_count);
  Q<producer_value> q;
  for (std::size_t i = 0; i < messages; ++i) {
    q.enqueue(producer_value{0, int(i + 1)});
    testset[0][i] = i + 1;
  }
  for (auto& f : valid_orders) {
    f = std::async(std::launch::async, [&]{ return consume(q, messages / consumers_count, 1); });
  }
  std::for_each(valid_orders.begin(), valid_orders.end(), [](auto& f){ REQUIRE(f.get()); });
  REQUIRE(q.empty());
}

} // namespace lockfree
