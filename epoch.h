#pragma once

#include <atomic>
#include <cassert>
#include <forward_list>
#include "tests/catch.hpp"

namespace lockfree {

template <typename T>
struct epoch {
  struct guard {
    // scan all processes to determine if they have observed the current epoch
    // fixme?: I am assuming freeing memory doesn't throw exception
    guard(epoch& e) noexcept : e_{e}, thread_epoch{e.global.load()} {
      // observe the current epoch
      e_.active[thread_epoch % epoch_count]++;
      // check if all running operation have observed the current epoch
      // and increment global epoch using CAS if they have
      if (e_.all_observed_epoch(thread_epoch) &&
          e_.global.compare_exchange_strong(thread_epoch, thread_epoch + 1))
      {
        e_.free_epoch(thread_epoch - 1);
      }
    }

    ~guard() noexcept {
      if (!unpinned) {
        unpin();
      }
    }

    void unpin() {
      // increment global epoch if this guard is guarding last operation in this epoch
      // or NOT I guess
      e_.active[thread_epoch % epoch_count]--;
      // fixme: leaking memory since deallocation takes place at the begining of the operation
      unpinned = true;
    }

    void unlink(T* pointer) {
      // fixme: boo! data race condition
      e_.local_unlinked[thread_epoch % epoch_count].emplace_front(pointer);
      //e_.unlinked[thread_epoch % epoch_count].emplace_front(pointer);
    }

  private:
    epoch& e_;
    unsigned thread_epoch;
    std::array<limbo_list, epoch_count> local_unlinked;
    bool unpinned{false};
  };

  guard pin() {
    return guard{*this};
  }

private:
  friend guard;

  using limbo_list = std::forward_list<std::unique_ptr<T>>;

  void free_epoch(unsigned n) {
    unlinked[n % epoch_count].clear();
  }

  bool all_observed_epoch(unsigned n) {
      return active[(n - 1) % epoch_count].load() == 0;
  }

  static constexpr unsigned epoch_count{3};
  std::atomic<unsigned> global{1};
  std::array<limbo_list, epoch_count> global_unlinked;
  thread_local std::array<limbo_list, epoch_count> local_unlinked;
  std::array<std::atomic<unsigned>, epoch_count> active;
};

TEST_CASE("Epoch - Basic test") {
  epoch<int> e;
  SECTION("") {
    auto g = e.pin();
  }
  SECTION("") {
    epoch<int>::guard g = e.pin();
  }
}

}
