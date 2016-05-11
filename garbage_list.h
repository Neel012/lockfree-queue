#pragma once

#include <atomic>
#include <memory>
#include <iostream>
#include "tests/catch.hpp"

namespace lockfree {

template <typename T>
struct garbage_list {
  struct node {
    node(T* ptr) : data{ptr} {}

    std::unique_ptr<T> data;
    node* next = nullptr;
  };

  ~garbage_list() noexcept {
    clear();
  }

  // Assumes rhs is not beeing accessed concurrently
  // Invariant: Epochs do not observe progress for the duration of this operation.
  void merge(garbage_list& rhs) {
    if (rhs.head_.load() == nullptr) {
      return;
    }
    while (true) {
      node* head = head_.load();
      rhs.tail_->next = head;
      if (head_.compare_exchange_weak(head, rhs.head_)) {
        break;
      }
    }
  }

  void emplace_back(T* ptr) {
    auto new_node = new node{ptr};
    if (tail_ == nullptr) {
      head_.store(new_node);
    } else {
      tail_->next = new_node;
    }
    tail_ = new_node;
  }

  void clear() noexcept {
    node* n = head_.load();
    if (n == nullptr) {
      return;
    }
    while (n->next != nullptr) {
      node* d = n;
      n = n->next;
      delete d;
    }
    delete n;
    head_.store(nullptr);
    tail_ = nullptr;
  }

private:
  std::atomic<node*> head_{nullptr};
  node* tail_{nullptr};
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

} // namespace tests
} // namespace lockfree
