#pragma once

#include <atomic>
#include <cassert>
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

  void enqueue(value_type&& value) {
    enqueue_(new node(std::move(value)));
  }

  optional dequeue() noexcept {
    assert(head_.load() != nullptr && tail_.load() != nullptr);
    optional value;
    epoch_guard g{epoch_};
    node* head = head_.load();
    node* tail = tail_.load();
    while (true) {
      node* next = head->next.load();
      if (head != head_) {
        head = head_.load();
      } else {
        if (head == tail) {
          if (next == nullptr) {
            return optional{};
          }
          tail_.compare_exchange_weak(tail, next);
          head = head_.load();
        } else {
          value = std::move(next->data);
          if (head_.compare_exchange_weak(head, next)) {
            break;
          }
        }
      }
    }
    g.unlink(head);
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

  void enqueue_(node* new_node) noexcept {
    node* tail = tail_.load();
    while (true) {
      if (tail != tail_) {
        tail = tail_.load();
      } else {
        node* next = tail->next.load();
        if (next == nullptr) {
          if (tail->next.compare_exchange_weak(next, new_node)) {
            break;
          }
          tail = tail_.load();
        } else {
          tail_.compare_exchange_weak(tail, next);
        }
      }
    }
    tail_.compare_exchange_weak(tail, new_node);
  }

  /* data */
  std::atomic<node*> head_;
  std::atomic<node*> tail_;
  epoch epoch_;
};

} // namespace lockfree
