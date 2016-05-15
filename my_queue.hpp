#pragma once

#include <atomic>
#include "queue.hpp"
#include <iostream>
#include <unistd.h>
namespace lockfree {

template<typename T>
struct my_queue : queue<T> {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  struct Node {
    Node(value_type &d) : value(d) { }
    Node(value_type &&d) : value(std::move(d)) { }

    /* data */
    value_type value;
    std::atomic<Node *> previous{nullptr};
  };

  std::atomic<Node *> back{nullptr};
  std::atomic<Node *> front{nullptr};


  void enqueue(value_type &value) final {
    enqueue_(new Node(value));
  }

  void enqueue(value_type &&value) final {
    enqueue_(new Node(std::move(value)));
  }

  ~my_queue() {
    while (dequeue()) { }
  }

  bool empty() {
    return front == nullptr;
  }

  void enqueue_(Node *node) {
    Node *old_back = back.exchange(node);
    if (old_back == nullptr) {
      front = node;
    } else {
      old_back->previous = node;
    }
  }

  optional dequeue() {
    optional value;
    Node *_front = front;
    Node *_previous;
    while (true) {
      if (_front == nullptr) {
        return optional();
      }
      _previous = _front->previous;
      if (front.compare_exchange_strong(_front, _front->previous)) {
        Node * __front = _front;
        back.compare_exchange_strong(__front, nullptr);

        if (!front && !_previous && _front->previous.load()) {
          front = _front->previous.load();
        }
        value = _front->value;
        delete _front;
        break;
      }
    }

    return value;
  }
};
}