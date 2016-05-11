#pragma once

#include "queue.h"
#include "epoch.h"

namespace lockfree {

template <typename T>
struct epoch_queue : queue<T> {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  epoch_queue() {
    auto* new_node = new node{};
    tail_.store(new_node);
    head_.store(new_node);
  }

  void enqueue(value_type& value) final {
    enqueue_(new node(value));
  }

  void enqueue(value_type&& value) final {
    enqueue_(new node(std::move(value)));
  }

  optional dequeue() final {
    optional value;
    node* head;
    epoch_guard<node> g{epoch_};
    while (true) {
      node* head = head_.load();
      node* tail = tail_.load();
      node* next = head->next.load();
      if (head == head_) {
        if (head == tail) {
          if (next == nullptr) {
            return optional();
          }
          tail_.compare_exchange_weak(tail, next);
        } else {
          value = next->data;
          if (head_.compare_exchange_weak(head, next)) {
            g.unlink(head);
            break;
          }
        }
      }
    }
    return value;
  }

private:
  struct node {
    node() = default;

    node(value_type& d) : data(d) {}

    node(value_type&& d) : data(std::move(d)) {}

    /* data */
    value_type data;
    std::atomic<node*> next{nullptr};
  };
  //using pointer_type = node*;

  void enqueue_(node* new_node) {
    node* tail;
    while (true) {
      tail = tail_.load();
      node* next = tail->next.load();
      if (tail == tail_) {
        if (next == nullptr) {
          if (tail_.compare_exchange_weak(tail, new_node)) {
            break;
          }
        } else {
          tail_.compare_exchange_weak(tail, next);
        }
      }
    }
    tail_.compare_exchange_weak(tail, new_node);
  }

  std::atomic<node*> head_;
  std::atomic<node*> tail_;
  epoch<node> epoch_;
};

namespace tests {

TEST_CASE("Epoch Queue - Basic tests - single thread") {
  epoch_queue<int> q;
  q.enqueue(1);
  q.enqueue(2);
  q.enqueue(3);
  REQUIRE(*q.dequeue() == 1);
  REQUIRE(*q.dequeue() == 2);
  REQUIRE(*q.dequeue() == 3);
}

} // namespace tests
} // namespace lockfree
