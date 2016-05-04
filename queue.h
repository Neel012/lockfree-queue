#pragma once

#include <experimental/optional>

namespace lockfree {

template <typename T>
struct queue {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  virtual void enqueue(value_type& value) = 0;

  virtual void enqueue(value_type&& value) = 0;

  virtual optional dequeue() = 0;

  virtual ~queue() {};

};

}
