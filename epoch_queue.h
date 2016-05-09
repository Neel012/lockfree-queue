#pragma once

#include "queue.h"
#include "epoch.h"

namespace lockfree {

template <typename T>
struct epoch_queue : queue<T> {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  void enqueue(value_type& value) final;

  void enqueue(value_type&& value) final;

  optional dequeue() final;

private:
  struct node {
    node() = default;

    node(value_type& d) : data(d) {}

    node(value_type&& d) : data(std::move(d)) {}

    /* data */
    value_type data;
    std::atomic<node*> next{nullptr};
  };
  using pointer_type = node*;
};

}
