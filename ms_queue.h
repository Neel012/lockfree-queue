#include "queue.h"

namespace lockfree {

template <typename T>
struct ms_queue : queue<T> {
  using value_type = T;
  using optional = std::experimental::optional<value_type>;

  void enqueue(value_type& value) final;

  void enqueue(value_type&& value) final;

  optional dequeue() final;

};

}
