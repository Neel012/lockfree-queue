#pragma once

#include <atomic>
#include <memory>
#include <iostream>
#include "atomic_ptr.hpp"

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

} // namespace lockfree
