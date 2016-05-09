#pragma once

#include <atomic>
#include <cassert>
#include <forward_list>
#include "list.h"
#include "tests/catch.hpp"

namespace lockfree {

static constexpr unsigned epoch_count{3};

template <typename T>
struct epoch;

template <typename T>
struct guard {
  using epoch = epoch<T>;
  using limbo_list = typename epoch::limbo_list;

  guard(epoch& e) noexcept : e_{e}, thread_epoch{e.global.load()} {
    e_.active[thread_epoch % epoch_count]++; // observe the current epoch
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

  void unlink(T* pointer) {
    e_.local_unlinked[thread_epoch % epoch_count].emplace_front(pointer);
  }

  void unpin() {
    // fixme: leaking some memory at the end of a lifetime of the epoch object
    // ... deallocation takes place at the begining of the guarded operation
    e_.active[thread_epoch % epoch_count]--;
    e_.merge_garbage(local_unlinked, thread_epoch);
    unpinned = true;
  }

private:
  /* data */
  epoch& e_;
  unsigned thread_epoch;
  limbo_list local_unlinked;
  bool unpinned{false};
};

// epoch manages pointers T
template <typename T>
struct epoch {
  //using limbo_list = std::forward_list<std::unique_ptr<T>>;
  using limbo_list = list<std::unique_ptr<T>>;
  //using limbo_list = list<T*>;
  using epoch_garbage = std::array<limbo_list, epoch_count>;

private:
  friend guard<T>;

  void free_epoch(unsigned n) {
    unlinked[n % epoch_count].clear();
  }

  bool all_observed_epoch(unsigned n) {
      return active[(n - 1) % epoch_count].load() == 0;
  }

  void merge_garbage(limbo_list& local_unlinked, unsigned local_epoch) {
    unlinked[local_epoch % epoch_count].merge(local_unlinked);
  }

  /* data */
  std::atomic<unsigned> global{1};
  std::array<std::atomic<unsigned>, epoch_count> active;
  epoch_garbage unlinked;
  //thread_local std::array<limbo_list, epoch_count> local_unlinked;
};

TEST_CASE("Epoch - Basic test") {
  epoch<int> e;
  SECTION("") {
    //auto g = e.pin();
  }
  SECTION("") {
    //epoch<int>::guard g = e.pin();
  }
  SECTION("") {
    //epoch<int>::guard g{e};
    guard<int> g{e};
  }
}

}
