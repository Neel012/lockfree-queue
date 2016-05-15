#pragma once

#include <atomic>
#include <memory>
#include <iostream>
#include <vector>
#include "atomic_ptr.hpp"
#include "any_ptr.hpp"

namespace lockfree {

struct garbage_stack;

// (Thread local) guard garbage stores type-erased managed pointers
struct garbage {
  garbage() = default;
  garbage(garbage&& g) noexcept
      : data{std::move(g.data)}, next{std::move(g.next)}
  { }

  void clear() noexcept {
    data.clear();
  }

  // void clear() noexcept {
  //   head_.reset();
  //   tail_ = nullptr;
  // }

  // bool empty() const noexcept {
  //   return head_ == nullptr;
  // }

  bool empty() const noexcept {
    return data.empty();
  }

  // Assumes *this is not beeing accessed concurrently
  // void emplace_back(T* ptr) {
  //   auto* new_node = new node{ptr};
  //   if (head_ == nullptr) {
  //     head_.reset(new_node);
  //   } else {
  //     tail_->next.reset(new_node);
  //   }
  //   tail_ = new_node;
  // }

  template <typename E>
  void emplace_back(E* ptr) {
    data.emplace_back(ptr);
  }

private:
  friend garbage_stack;
  // std::unique_ptr<node> head_{nullptr};
  // node* tail_{nullptr};
  std::vector<any_ptr> data;
  std::unique_ptr<garbage> next{nullptr};
};

// Global garbage_stack is used to store managed pointers and is accessed
// concurrently. 
struct garbage_stack {

  void clear() {
    head_.reset();
  }

  // Assumes g is not beeing accessed concurrently
  // Invariant: Epochs do not observe progress for the duration of this operation.
  //void merge(garbage<T>& g) {
  //  if (g.empty()) {
  //    return;
  //  }
  //  while (true) {
  //    node* head = head_.load();
  //    g.tail_->next.reset(head);
  //    if (head_.compare_exchange_weak(head, g.head_.get())) {
  //      g.head_.release();
  //      break;
  //    }
  //    g.tail_->next.release();
  //  }
  //}

  void merge(garbage& g) {
    if (g.empty()) {
      return;
    }
    auto* new_node = new garbage{std::move(g)};
    while (true) {
      garbage* head = head_.load();
      new_node->next.reset(head);
      if (head_.compare_exchange_weak(head, new_node)) {
        break;
      }
      new_node->next.release();
    }
  }

private:
  atomic_ptr<garbage> head_{nullptr};
};

} // namespace lockfree
