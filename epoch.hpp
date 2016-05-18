#pragma once

#include <atomic>
#include <cassert>
#include <array>
#include "garbage.hpp"

namespace lockfree {

static constexpr unsigned epoch_count{3};

struct epoch_guard;

// epoch manages pointers
struct epoch {
private:
  friend epoch_guard;

  void free_epoch(unsigned n) noexcept {
    assert(active_[n % epoch_count].load() == 0);
    unlinked_[n % epoch_count].clear();
  }

  bool all_observed_epoch(unsigned n) const noexcept {
    return active_[(n - 1) % epoch_count].load() == 0;
  }

  bool progress_epoch(unsigned guard_epoch) noexcept {
    return global_epoch_.compare_exchange_strong(guard_epoch, guard_epoch + 1);
  }

  void merge_garbage(garbage&& local_unlinked, unsigned local_epoch) noexcept {
    unlinked_[local_epoch % epoch_count].merge(local_unlinked);
  }

  /* data */
  std::atomic<unsigned> global_epoch_{1};
  std::array<std::atomic<unsigned>, epoch_count> active_;
  std::array<garbage_stack, epoch_count> unlinked_;
};

struct epoch_guard {

  explicit epoch_guard(epoch& e) noexcept
    : e_{e}, guard_epoch_{e.global_epoch_.load()}
  {
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

  template <typename E>
  void unlink(E* ptr) noexcept {
    local_unlinked_.emplace_back(ptr);
  }

  void unpin() noexcept {
    e_.merge_garbage(std::move(local_unlinked_), guard_epoch_);
    e_.active_[guard_epoch_ % epoch_count]--;
    unpinned_ = true;
  }

private:
  /* data */
  epoch& e_;
  unsigned guard_epoch_;
  garbage local_unlinked_;
  bool unpinned_{false};
};

} // namespace lockfree
