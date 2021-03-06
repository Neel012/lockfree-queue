#pragma once

#include <atomic>
#include <cassert>
#include <iostream>
#include <memory>
#include <vector>

#include <atomic_ptr.hpp>

namespace lockfree {

struct garbage_stack;

// guard garbage stores type-erased managed pointers
struct garbage {
  garbage() = default;
  garbage(const garbage&) = delete;
  garbage(garbage&&) = default;

  garbage& operator=(const garbage&) = delete;
  garbage& operator=(garbage&&) = default;

  void clear() noexcept {
    data.clear();
  }

  bool empty() const noexcept {
    return data.empty();
  }

  template <typename E>
  void emplace_back(E* ptr) {
    data.emplace_back(any_ptr{ptr, [](void* d) { delete static_cast<E*>(d); }});
  }

private:
  friend garbage_stack;

  using deleter_type = void(*)(void*);
  using any_ptr = std::unique_ptr<void, deleter_type>;

  std::vector<any_ptr> data;
  garbage* next{nullptr};
};

// Global garbage_stack is used to store managed pointers and is accessed
// concurrently. 
struct garbage_stack {

  ~garbage_stack() noexcept {
    clear();
  }

  void clear() noexcept {
    garbage* node = head_.release();
    while (node != nullptr) {
      garbage* d = node;
      node = node->next;
      delete d;
    }
  }

  void merge(garbage& g) {
    if (g.empty()) {
      return;
    }
    auto* new_node = new garbage{std::move(g)};
    assert(new_node->next == nullptr);
    garbage* head = head_.load();
    while (true) {
      new_node->next = head;
      if (head_.compare_exchange_weak(head, new_node)) {
        break;
      }
    }
  }

private:
  atomic_ptr<garbage> head_{nullptr};
};

} // namespace lockfree
