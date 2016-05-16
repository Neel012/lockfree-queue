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
    volatile Node * previous{nullptr};
  };

  std::atomic<volatile Node *> back{nullptr};
  std::atomic<volatile Node *> front{nullptr};


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
    volatile Node *old_back = back.exchange(node);
    if (old_back == nullptr) {
      front = node;
    } else {
      old_back->previous = node;
    }
  }

//  mene lock-free verze, ale funguje na 100%.
  optional dequeue() {
    volatile Node *old_front = front.exchange(nullptr);
    if (!old_front) {
      return optional();
    }
    volatile Node * _front = old_front;
    if (!back.compare_exchange_strong(_front, nullptr)) {
      while (true) {
        _front = old_front->previous;
        if (_front) break;
      }
      front = _front;
    }

    optional value = static_cast<int>(old_front->value);
    delete old_front;
    return value;
  }
// //  vice lock-free verze, ale funguje na 80% (někdy, v jednom benchmarku spadne, ale.. skoro bez chyby)
//  optional dequeue() {
//    optional value;
//    Node *_front = front;
//    Node *_previous;
//    while (true) {
//      if (_front == nullptr) {
//        return optional();
//      }
//      _previous = _front->previous;
//      if (front.compare_exchange_strong(_front, _front->previous)) {
//        Node * __front = _front;
//        if (!back.compare_exchange_strong(__front, nullptr)) {
//          if (!front && !_previous) {
//            front = _front->previous.load();
//          }
//        }
//
//        value = _front->value;
//        delete _front;
//        break;
//      }
//    }
//
//    return value;
//  }
};
}