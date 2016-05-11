#pragma once

#include <atomic>
#include <cassert>
#include "garbage_list.h"
#include "tests/catch.hpp"

namespace lockfree {

static constexpr unsigned epoch_count{3};

template <typename T>
struct epoch;

template <typename T>
struct epoch_guard {
  using epoch = epoch<T>;
  using limbo_list = typename epoch::limbo_list;

  epoch_guard(epoch& e) noexcept : e_{e}, guard_epoch_{e.global_epoch_.load()} {
    e_.active_[guard_epoch_ % epoch_count]++; // observe the current epoch
    if (e_.all_observed_epoch(guard_epoch_) &&
        e_.global_epoch_.compare_exchange_strong(guard_epoch_, guard_epoch_ + 1))
    {
      e_.free_epoch(guard_epoch_ - 1);
    }
  }

  ~epoch_guard() noexcept {
    if (!unpinned_) {
      unpin();
    }
  }

  void unlink(T* pointer) {
    e_.local_unlinked[guard_epoch_ % epoch_count].emplace_front(pointer);
  }

  void unpin() {
    // fixme: leaking some memory at the end of a lifetime of the epoch object
    // ... deallocation takes place at the begining of the guarded operation
    e_.active_[guard_epoch_ % epoch_count]--;
    e_.merge_garbage(local_unlinked_, guard_epoch_);
    unpinned_ = true;
  }

private:
  /* data */
  epoch& e_;
  unsigned guard_epoch_;
  limbo_list local_unlinked_;
  bool unpinned_{false};
};

// epoch manages pointers T
template <typename T>
struct epoch {
  using limbo_list = garbage_list<T>;
  using epoch_garbage = std::array<limbo_list, epoch_count>;

private:
  friend epoch_guard<T>;

  void free_epoch(unsigned n) {
    unlinked_[n % epoch_count].clear();
  }

  bool all_observed_epoch(unsigned n) {
    return active_[(n - 1) % epoch_count].load() == 0;
  }

  void merge_garbage(limbo_list& local_unlinked, unsigned local_epoch) {
    unlinked_[local_epoch % epoch_count].merge(local_unlinked);
  }

  /* data */
  std::atomic<unsigned> global_epoch_{1};
  std::array<std::atomic<unsigned>, epoch_count> active_;
  epoch_garbage unlinked_;
  //thread_local std::array<limbo_list, epoch_count> local_unlinked;
};

namespace tests {

TEST_CASE("Epoch - Basic test") {
  epoch<int> e;
  SECTION("") {
    epoch_guard<int> g{e};
  }
}

} // namespace tests
} // namespace lockfree
