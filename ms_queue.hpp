#pragma once

#include <atomic>
#include <cassert>
#include <experimental/optional>

#include <tagged_pointer.hpp>

namespace lockfree {

template <typename T>
struct ms_queue {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  ms_queue() {
    pointer_type new_node(new node{});
    tail_.store(new_node);
    head_.store(new_node);
  }

  ~ms_queue() noexcept {
    node* n = head_.exchange(nullptr).ptr();
    while (n != nullptr) {
      node* d = n;
      n = n->next.load().ptr();
      delete d;
    }
  }

  bool empty() const noexcept {
    return head_.load().ptr()->next.load().ptr() == nullptr;
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
    pointer_type head = head_.load();
    pointer_type tail = tail_.load();
    while (true) {
      pointer_type next = head.ptr()->next.load();
      if (head != head_) {
        head = head_.load();
      } else {
        if (head.ptr() == tail.ptr()) {
          if (next.ptr() == nullptr) {
            return optional{};
          }
          tail_.compare_exchange_weak(tail, pointer_type(next.ptr(), tail.count() + 1));
          head = head_.load();
        } else {
          value = std::move(next.ptr()->data);
          if (head_.compare_exchange_weak(head, pointer_type(next.ptr(), head.count() + 1))) {
            break;
          }
        }
      }
    }
    delete head.ptr();
    return value;
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

  void enqueue_(node* new_node) noexcept {
    pointer_type tail = tail_.load();
    while (true) {
      if (tail != tail_) {
        tail = tail_;
      } else {
        pointer_type next = tail.ptr()->next.load();
        if (next.ptr() == nullptr) {
          if (tail.ptr()->next.compare_exchange_weak(next, pointer_type(new_node, next.count() + 1))) {
            break;
          }
          tail = tail_.load();
        } else {
          tail_.compare_exchange_weak(tail, pointer_type(next.ptr(), tail.count() + 1));
        }
      }
    }
    tail_.compare_exchange_weak(tail, pointer_type(new_node, tail.count() + 1));
  }

  /* data */
  std::atomic<pointer_type> head_;
  std::atomic<pointer_type> tail_;
};

} // namespace lockfree
