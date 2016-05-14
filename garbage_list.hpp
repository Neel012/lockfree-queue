#pragma once

#include <atomic>
#include <memory>
#include <iostream>
#include "atomic_ptr.hpp"
#include "tests/catch.hpp"

namespace lockfree {

template <typename T>
struct garbage;

// Thread local garbage
template <typename T>
struct garbage_list {
  struct node {
    node(T* ptr) : data{ptr} {}

    std::unique_ptr<T> data;
    std::unique_ptr<node> next{nullptr};
  };

  void clear() {
    head_.reset();
    tail_ = nullptr;
  }

  bool empty() const {
    return head_ == nullptr;
  }

  // Assumes *this is not beeing accessed concurrently
  void emplace_back(T* ptr) {
    auto* new_node = new node{ptr};
    if (head_ == nullptr) {
      head_.reset(new_node);
    } else {
      tail_->next.reset(new_node);
    }
    tail_ = new_node;
  }

private:
  friend garbage<T>;
  std::unique_ptr<node> head_{nullptr};
  node* tail_{nullptr};
};

// Global garbage
template <typename T>
struct garbage {
  using node = typename garbage_list<T>::node;

  void clear() {
    head_.reset();
  }

  // Assumes g is not beeing accessed concurrently
  // Invariant: Epochs do not observe progress for the duration of this operation.
  void merge(garbage_list<T>& g) {
    if (g.empty()) {
      return;
    }
    while (true) {
      node* head = head_.load();
      g.tail_->next.reset(head);
      if (head_.compare_exchange_weak(head, g.head_.get())) {
        g.head_.release();
        break;
      }
      g.tail_->next.release();
    }
  }

private:
  atomic_ptr<node> head_{nullptr};
};

namespace tests {

struct test_counter {
  test_counter(int& i) : n{i} { n++; }

  ~test_counter() { n--; }

  int& n;
};

TEST_CASE("Garbage List - Basic test") {
  int counter{0};
  SECTION("Proper deallocation") {
    garbage_list<test_counter> l;
    for (int i{0}; i < 20; i++) {
      l.emplace_back(new test_counter{counter});
    }
    l.clear();
    REQUIRE(counter == 0);
  }
  SECTION("Proper deallocation 2") {
    std::vector<test_counter*> vec;
    for (int i{0}; i < 10; i++) {
      vec.push_back(new test_counter{counter});
    }
    garbage_list<test_counter> l;
    for (int i{0}; i < 10; i++) {
      l.emplace_back(vec[i]);
    }
    l.clear();
    REQUIRE(counter == 0);
  }
}

TEST_CASE("Garbage - Basic test") {
  int counter{0};

  SECTION("merge one list") {
    garbage<test_counter> g;
    constexpr unsigned n = 3;
    {
      garbage_list<test_counter> l;
      for (unsigned i{0}; i < n; i++) {
        l.emplace_back(new test_counter{counter});
      }
      REQUIRE(counter == n);
      g.merge(l);
      REQUIRE(counter == n);
    }
    REQUIRE(counter == n);
  }

  SECTION("Proper deallocation") {
    garbage<test_counter> g;
    {
      garbage_list<test_counter> l;
      for (int i{0}; i < 20; i++) {
        l.emplace_back(new test_counter{counter});
      }
      garbage_list<test_counter> m;
      for (int i{0}; i < 20; i++) {
        m.emplace_back(new test_counter{counter});
      }
      g.merge(l);
      g.merge(m);
    }
    g.clear();
    REQUIRE(counter == 0);
  }
}

} // namespace tests
} // namespace lockfree
