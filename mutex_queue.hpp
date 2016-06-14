#include <experimental/optional>
#include <mutex>
#include <queue>

namespace lockfree {

  template<typename T>
  struct mutex_queue {
    using value_type = T;
    using optional = std::experimental::optional<value_type>;

    void enqueue(value_type &value) {
      std::lock_guard<std::mutex> l(mutex);
      queue.push(value);
    }

    void enqueue(value_type &&value) {
      std::lock_guard<std::mutex> l(mutex);
      queue.push(std::move(value));
    }

    optional dequeue() {
      optional result;

      std::lock_guard<std::mutex> l(mutex);
      if (!queue.empty()) {
        result = queue.front();
        queue.pop();
      }
      return result;
    }

    bool empty() const {
      std::lock_guard<std::mutex> l(mutex);
      queue.empty();
    }

  private:

    /* data */
    std::queue<value_type> queue;
    mutable std::mutex mutex;
  };


}

