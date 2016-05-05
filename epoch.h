#pragma once

#include <atomic>
#include <cassert>
#include <forward_list>
#include "tests/catch.hpp"

namespace lockfree {

template <typename T>
struct epoch {
  using limbo_list = std::forward_list<std::unique_ptr<T>>;

  struct guard {
    guard(epoch& e) noexcept : e_{e} {
      thread_epoch = e_.global;
      // scan all processes to determine if they have observed the current epoch
    }

    void unlink(T* pointer) {
      e_.unlinked[thread_epoch % epoch_count].emplace_front(pointer);
    }

    ~guard() {
      // increment global epoch if this guard is guarding last operation in this epoch
    }

  private:
    epoch& e_;
    unsigned thread_epoch;
  };

  void free_epoch(unsigned n) {
    //assert(0 <= n && n < 3);
    unlinked[n % epoch_count].clear();
  }

  guard pin() {
    return guard{*this};
  }

  static constexpr unsigned epoch_count = 3;
  std::atomic<unsigned> global {0};
  std::array<limbo_list, epoch_count> unlinked;
};

TEST_CASE("Epoch - Basic test") {
  epoch<int> e;
  e.pin();
}

}
