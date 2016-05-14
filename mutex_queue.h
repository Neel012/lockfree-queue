#include "queue.h"
#include <queue>

namespace lockfree {

  template<typename T>
  struct mutex_queue : queue<T> {
    using value_type = T;
    using optional = std::experimental::optional<value_type>;

    void enqueue(value_type &value) final {
      mutex.lock();
      queue.push(value);
      mutex.unlock();
    }

    void enqueue(value_type &&value) final {
      mutex.lock();
      queue.push(std::move(value));
      mutex.unlock();
    }

    optional dequeue() final {
      optional result;

      mutex.lock();
      if (!queue.empty()) {
        result = queue.front();
        queue.pop();
      }
      mutex.unlock();
      return result;
    }

  private:

    /* data */
    std::queue<value_type> queue;
    std::mutex mutex;
  };


}

