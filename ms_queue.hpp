#pragma once

#include <atomic>
#include "queue.hpp"
#include "tagged_pointer.hpp"
#include "tests/catch.hpp"

namespace lockfree {

template <typename T>
struct ms_queue : queue<T> {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  ms_queue() {
    pointer_type new_node(new node());
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
    pointer_type head;
    while (true) {
      head = head_.load();
      pointer_type tail = tail_.load();
      pointer_type next = head.ptr()->next.load();
      if (head == head_) {
        if (head.ptr() == tail.ptr()) {
          if (next.ptr() == nullptr) {
            return optional();
          }
          tail_.compare_exchange_weak(tail, pointer_type(next.ptr(), tail.count() + 1));
        } else {
          value = next.ptr()->data;
          if (head_.compare_exchange_weak(head, pointer_type(next.ptr(), head.count() + 1))) {
            break;
          }
        }
      }
    }
    delete head.ptr();
    return value;
  }

  ~ms_queue() {
    while (dequeue()) { }
  }

private:
  struct node;
  using pointer_type = tagged_pointer<node>;

  struct node {
    node() = default;
    node(value_type& d) : data(d) {}
    node(value_type&& d) : data(std::move(d)) {}

    /* data */
    value_type data;
    std::atomic<pointer_type> next{nullptr};
  };

  void enqueue_(node* new_node) {
    pointer_type tail;
    while (true) {
      tail = tail_.load();
      pointer_type next = tail.ptr()->next.load();
      if (tail == tail_) {
        if (next.ptr() == nullptr) {
          if (tail.ptr()->next.compare_exchange_weak(next, pointer_type(new_node, next.count() + 1))) {
            break;
          }
        } else {
          tail_.compare_exchange_weak(tail, pointer_type(next.ptr(), tail.count()));
        }
      }
    }
    tail_.compare_exchange_weak(tail, pointer_type(new_node, tail.count()));
  }

  /* data */
  std::atomic<pointer_type> head_;
  std::atomic<pointer_type> tail_;
};

} // namespace lockfree
