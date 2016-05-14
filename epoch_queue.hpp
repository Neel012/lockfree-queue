#pragma once

#include "queue.hpp"
#include "epoch.hpp"
#include "atomic_ptr.hpp"
#include "tests/catch.hpp"

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

  // ~epoch_queue() noexcept final {
  //   node* head = head_.load();
  // node* tail = tail_.load();
   // TODO: deallocate inserted nodes + dummy node
  // }

  void enqueue(value_type& value) final {
    enqueue_(new node(value));
  }

  void enqueue(value_type&& value) final {
    enqueue_(new node(std::move(value)));
  }

  optional dequeue() final {
    while (true) {
      epoch_guard<node> g{epoch_};
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
    //std::atomic<node*> next{nullptr};
    atomic_ptr<node> next{nullptr};
  };
  //using pointer_type = node*;

  void enqueue_(node* new_node) {
    while (true) {
      node* tail = tail_.load();
      node* next = tail->next.load();
      if (next == nullptr) {
        if (tail->next.compare_exchange_weak(tail, new_node)) {
          tail_.compare_exchange_weak(tail, new_node);
          return;
        }
      } else {
        tail_.compare_exchange_weak(tail, next);
      }
    }
  }

  //std::atomic<node*> head_;
  //std::atomic<node*> tail_;
  atomic_ptr<node> head_;
  atomic_ptr<node> tail_;
  epoch<node> epoch_;
};

} // namespace lockfree
