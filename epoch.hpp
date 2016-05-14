#pragma once

#include <atomic>
#include <cassert>
#include "garbage_list.hpp"
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
    if (e_.all_observed_epoch(guard_epoch_) && e_.progress_epoch(guard_epoch_))
    {
      e_.free_epoch(guard_epoch_ - 1);
    }
  }

  ~epoch_guard() noexcept {
    if (!unpinned_) {
      unpin();
    }
  }

  void unlink(T* pointer) noexcept {
    local_unlinked_.emplace_back(pointer);
  }

  void unpin() noexcept {
    // fixme: leaking some memory at the end of a lifetime of the epoch object
    // ... deallocation takes place at the begining of the guarded operation
    e_.merge_garbage(local_unlinked_, guard_epoch_);
    e_.active_[guard_epoch_ % epoch_count]--;
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
  using epoch_garbage = std::array<garbage<T>, epoch_count>;

private:
  friend epoch_guard<T>;

  void free_epoch(unsigned n) noexcept {
    unlinked_[n % epoch_count].clear();
  }

  bool all_observed_epoch(unsigned n) noexcept {
    return active_[(n - 1) % epoch_count].load() == 0;
  }

  bool progress_epoch(unsigned guard_epoch) noexcept {
    return global_epoch_.compare_exchange_strong(guard_epoch, guard_epoch + 1);
  }

  void merge_garbage(limbo_list& local_unlinked, unsigned local_epoch) noexcept {
    unlinked_[local_epoch % epoch_count].merge(local_unlinked);
  }

  /* data */
  std::atomic<unsigned> global_epoch_{1};
  std::array<std::atomic<unsigned>, epoch_count> active_;
  epoch_garbage unlinked_;
  //thread_local std::array<limbo_list, epoch_count> local_unlinked;
};

} // namespace lockfree
