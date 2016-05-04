#include <atomic>
#include "queue.h"
#include "tests/catch.hpp"

namespace lockfree {

template <typename T>
struct ms_queue : queue<T> {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  ms_queue() {
    node* new_node = new node();
    tail_ = new_node;
    head_ = new_node;
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
    while (true) {
      head = head_.load();
      node* tail = tail_.load();
      node* next = head->next.load();
      if (head == head_) {
        if (head == tail) {
          if (next == nullptr) {
            return optional();
          }
          tail_.compare_exchange_strong(tail, next);
        } else {
          value = next->data;
          if (head_.compare_exchange_weak(head, next)) {
            break;
          }
        }
      }
    }
    // fixme?: check if head is nullptr
    delete head;
    return value;
  }

private:
  struct node {
    node() noexcept : next(nullptr) {}

    node(value_type& d) : data(d), next(nullptr) {}

    node(value_type&& d) : data(std::move(d)), next(nullptr) {}

    /* data */
    value_type data;
    std::atomic<node*> next;
  };

  void enqueue_(node* new_node) {
    node* tail;
    while (true) {
      tail = tail_.load();
      node* next = tail->next.load();
      if (tail == tail_) {
        if (next == nullptr) {
          if (tail->next.compare_exchange_weak(next, new_node)) {
            break;
          }
        } else {
          tail_.compare_exchange_strong(tail, next);
        }
      }
    }
    tail_.compare_exchange_strong(tail, new_node);
  }

  /* data */
  std::atomic<node*> head_;
  std::atomic<node*> tail_;
};

TEST_CASE("Basic tests - single thread") {
  ms_queue<int> q;
  q.enqueue(1);
  q.enqueue(2);
  q.enqueue(3);
  REQUIRE(*q.dequeue() == 1);
  REQUIRE(*q.dequeue() == 2);
  REQUIRE(*q.dequeue() == 3);
}

}
