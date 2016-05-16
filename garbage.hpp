#pragma once

#include <atomic>
#include <memory>
#include <iostream>
#include <vector>
#include "atomic_ptr.hpp"
#include "any_ptr.hpp"

namespace lockfree {

struct garbage_stack;

// guard garbage stores type-erased managed pointers
struct garbage {
  garbage() = default;
  garbage(garbage&& g) noexcept
      : data{std::move(g.data)}, next{std::move(g.next)}
  { }

  void clear() noexcept {
    data.clear();
  }

  bool empty() const noexcept {
    return data.empty();
  }

  template <typename E>
  void emplace_back(E* ptr) {
    data.emplace_back(ptr);
  }

private:
  friend garbage_stack;
  std::vector<any_ptr> data;
  std::unique_ptr<garbage> next{nullptr};
};

// Global garbage_stack is used to store managed pointers and is accessed
// concurrently. 
struct garbage_stack {

  void clear() {
    head_.reset();
  }

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
