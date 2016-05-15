#pragma once

#include "epoch.hpp"
#include "queue.hpp"
#include "atomic_ptr.hpp"

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

  bool empty() const noexcept {
    return head_.load()->next == nullptr;
  }

  void enqueue(value_type& value) final {
    enqueue_(new node(value));
  }

  void enqueue(value_type&& value) final {
    enqueue_(new node(std::move(value)));
  }

  optional dequeue() noexcept final {
    while (true) {
      epoch_guard g{epoch_};
      node* head = head_.load();
      node* tail = tail_.load();
      node* next = head->next.load();
      if (head == tail) {
        if (next == nullptr) {
          return optional();
        }
        tail_.compare_exchange_weak(tail, next);
      } else {
        if (head_.compare_exchange_weak(head, next)) {
          // fixme: verify that i can modify the node->next?
          head->next.release();
          g.unlink(head);
          return next->data;
        }
      }
    }
  }

private:
  struct node {
    node() = default;
    node(value_type& d) : data(d) {}
    node(value_type&& d) : data(std::move(d)) {}

    /* data */
    value_type data;
    atomic_ptr<node> next{nullptr};
  };

  void enqueue_(node* new_node) noexcept {
    while (true) {
      node* tail = tail_.load();
      node* next = tail->next.load();
      if (next == nullptr) {
        if (tail->next.compare_exchange_weak(next, new_node)) {
          tail_.compare_exchange_weak(tail, new_node);
          return;
        }
      } else {
        tail_.compare_exchange_weak(tail, next);
      }
    }
  }

  /* data */
  atomic_ptr<node> head_;
  std::atomic<node*> tail_;
  epoch epoch_;
};

} // namespace lockfree
