#pragma once

#include <experimental/optional>

#include <epoch.hpp>

namespace lockfree {

template <typename T>
struct epoch_queue {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  epoch_queue() {
    auto* new_node = new node{};
    tail_.store(new_node);
    head_.store(new_node);
  }

  ~epoch_queue() noexcept {
    node* n = head_.load();
    while (n != nullptr) {
      node* d = n;
      n = n->next.load();
      delete d;
    }
  }

  bool empty() const noexcept {
    return head_.load()->next == nullptr;
  }

  void enqueue(value_type& value) noexcept {
    enqueue_(new node(value));
  }

  void enqueue(value_type&& value) noexcept {
    enqueue_(new node(std::move(value)));
  }

  optional dequeue() noexcept {
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
    std::atomic<node*> next{nullptr};
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
  std::atomic<node*> head_;
  std::atomic<node*> tail_;
  epoch epoch_;
};

} // namespace lockfree
