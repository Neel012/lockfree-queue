#pragma once

#include <future>
#include <thread>

#include <tests/catch.hpp>

namespace lockfree {

struct test_counter {
  test_counter(int& i) : n{i} { n++; }

  ~test_counter() { n--; }

  int& n;
};

template <template <typename> class Q>
void test_queue_basic(Q<int>& q, size_t count) {
  REQUIRE(q.empty());
  for (size_t i{0}; i < count; i++) {
    q.enqueue(i + 1);
  }
  REQUIRE(!q.empty());
  for (size_t i{0}; i < count; i++) {
    REQUIRE(*q.dequeue() == i + 1);
  }
  REQUIRE(q.empty());
}

template <template <typename> class Q>
void test_only_enqueue(Q<int>& q, size_t count) {
  REQUIRE(q.empty());
  for (size_t i{0}; i < count; i++) {
    q.enqueue(i + 1);
  }
  REQUIRE(!q.empty());
}

template <typename Q>
void maybe_recieve(Q& q, size_t count) {
  int i = 1;
  while (i <= count) {
    auto pop = q.dequeue();
    if (pop) {
      REQUIRE(*pop == i);
      i++;
    }
  }
}

template <typename Q>
void maybe_recieve_anything(Q& q, size_t count) {
  int i = 1;
  while (i <= count) {
    if (q.dequeue()) {
      i++;
    }
  }
}

template <template <typename> class Q>
void test_queue_basic2(Q<int>& q, size_t count) {
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
void test_queue_t2(Q<int>& q, size_t n) {
  auto t = std::thread([&] {
    for (size_t i{0}; i < n; i++) {
      q.enqueue(i + 1);
    }
  });
  maybe_recieve(q, n);
  t.join();
  REQUIRE(q.empty());
}

template <template <typename> class Q>
void test_queue_t3(Q<int>& q, size_t n) {
  auto t1 = std::thread([&] { maybe_recieve_anything(q, n); });
  auto t2 = std::thread([&] {
    for (std::size_t i{0}; i < n/2; i++) {
      q.enqueue(i + 1);
    }
  });
  for (std::size_t i{0}; i < (n / 2) + (n % 2); i++) {
    q.enqueue(i + 1);
  }
  t1.join();
  t2.join();
  REQUIRE(q.empty());
}

struct producer_value {
  unsigned producer;
  int value;
};

template <typename Q>
void produce(Q& q, unsigned producer_num, std::vector<int>& testset) {
  for (auto& v : testset) {
    q.enqueue(producer_value{producer_num, v});
  }
}

template <typename Q>
std::vector<producer_value> consume(Q& q, std::size_t count) {
  std::vector<producer_value> received(count);
  for (std::size_t i{0}; i < count;) {
    auto r = q.dequeue();
    if (r) {
      received[i] = *r;
      ++i;
    }
  }
  return received;
}

std::vector<std::vector<int>> gen_seqset(unsigned producers_count, std::size_t chunk_size) {
  std::vector<std::vector<int>> testset{producers_count, std::vector<int>(chunk_size)};
  unsigned i{1};
  for (auto& s : testset) {
    std::generate_n(s.begin(), chunk_size, [&i]{ return i++; });
  }
  return testset;
}

void check_testset(
    std::vector<std::vector<int>>& testset,
    std::vector<std::vector<producer_value>>& received,
    unsigned messages)
{
  const unsigned producers_count = testset.size();
  const unsigned consumers_count = received.size();
  const std::size_t producer_chunk = messages / producers_count;
  const std::size_t consumer_chunk = messages / consumers_count;
  std::vector<std::size_t> producer_idx(producers_count, 0);
  std::vector<std::size_t> consumer_idx(consumers_count, 0);

  unsigned msg_processed{0};
  for (unsigned msg_counter{1}; msg_counter <= messages; ++msg_counter) {
    REQUIRE(std::any_of(
        consumer_idx.begin(),
        consumer_idx.end(),
        [consumer_chunk](std::size_t i) { return i < consumer_chunk; }
    ));
    REQUIRE(std::any_of(
        producer_idx.begin(),
        producer_idx.end(),
        [producer_chunk](std::size_t i) { return i < producer_chunk; }
    ));
    for (unsigned i{0}; i < consumers_count; ++i) {
      if (consumer_idx[i] >= consumer_chunk) {
        continue;
      }
      producer_value pv = received[i][consumer_idx[i]];
      if (testset[pv.producer][producer_idx[pv.producer]] == pv.value) {
        ++consumer_idx[i];
        ++producer_idx[pv.producer];
        ++msg_processed;
        break;
      }
    }
    REQUIRE(msg_counter == msg_processed); // One or more messages are not in allowed order
  }
  REQUIRE(std::all_of(
      consumer_idx.begin(),
      consumer_idx.end(),
      [consumer_chunk](std::size_t i) { return i == consumer_chunk; }
  ));
  REQUIRE(std::all_of(
      producer_idx.begin(),
      producer_idx.end(),
      [producer_chunk](std::size_t i) { return i == producer_chunk; }
  ));
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
void test_queue_manythreads(unsigned producers_count, unsigned consumers_count) {
  const std::size_t messages = remainderless(10'000, lcm(producers_count, consumers_count));
  std::vector<std::vector<int>> testset = gen_seqset(producers_count, messages / producers_count);
  std::vector<std::thread> producers{producers_count};
  std::vector<std::future<std::vector<producer_value>>> received(consumers_count);
  Q<producer_value> q;

  for (unsigned i{0}; i < producers_count; ++i) {
    producers[i] = std::thread(&produce<decltype(q)>, std::ref(q), i, std::ref(testset[i]));
  }
  for (auto& f : received) {
    f = std::async(std::launch::async, &consume<decltype(q)>, std::ref(q), messages / consumers_count);
  }
  for (auto& t : producers) {
    t.join();
  }
  for (auto& f : received) {
    f.wait();
  }
  REQUIRE(q.empty());
  std::vector<std::vector<producer_value>> received_unpack(consumers_count);
  std::transform(received.begin(), received.end(), received_unpack.begin(),
      [](auto& f){ return f.get(); });
  check_testset(testset, received_unpack, messages);
}

} // namespace lockfree
