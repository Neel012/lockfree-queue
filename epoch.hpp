#pragma once

#include <array>
#include <atomic>
#include <cassert>

#include <garbage.hpp>

// How to use epoch memory reclamation:
// All you have to do is add an epoch object to you data  structure to hold a
// state of epoch's garbages. Concurrent operations removing pointers from
// datastrucutres need to use an `epoch_quard` and call its unlink method on
// removed pointers. Unlinked pointers will be freed only after no other
// concurrent operations hold  references to them.
//
// Example:
// void pop_front() {
//   epoch_guard g;
//   old_front <- remove pointer from data structure and update it using
//                lockfree primitives
//   g.unlink(old_front);
// }

namespace lockfree {

namespace {

constexpr unsigned EPOCH_COUNT{3};

unsigned inc_epoch(unsigned n) noexcept {
  return (n+1) % EPOCH_COUNT;
}

unsigned dec_epoch(unsigned n) noexcept {
  return (n-1) % EPOCH_COUNT;
}

}

struct epoch_guard;

struct epoch {
private:
  friend epoch_guard;

  void free_epoch(unsigned n) noexcept {
#ifndef NDEBUG
    unsigned snapshot_active = active_[n];
#endif
    assert(snapshot_active == 0);
    unlinked_[n].clear();
  }

  bool all_observed_epoch(unsigned n) const noexcept {
#ifndef NDEBUG
    unsigned snapshot_global_epoch = global_epoch_;
#endif
    assert(snapshot_global_epoch == n || snapshot_global_epoch == inc_epoch(n));
    return active_[dec_epoch(n)] == 0;
  }

  bool progress_epoch(unsigned current_epoch) noexcept {
    assert(current_epoch < 3 && current_epoch >= 0);
    return global_epoch_.compare_exchange_strong(current_epoch, inc_epoch(current_epoch));
  }

  void merge_garbage(garbage&& local_unlinked, unsigned local_epoch) noexcept {
    unlinked_[local_epoch].merge(local_unlinked);
    assert(local_unlinked.empty());
  }

  /* data */
  std::atomic<unsigned> global_epoch_{1};
  std::array<std::atomic<unsigned>, EPOCH_COUNT> active_;
  std::array<garbage_stack, EPOCH_COUNT> unlinked_;
};

struct epoch_guard {

  explicit epoch_guard(epoch& e) noexcept
    : e_{e}, guard_epoch_{e.global_epoch_.load()}
  {
    assert(guard_epoch_ < 3 && guard_epoch_ >= 0);
    e_.active_[guard_epoch_]++; // observe the current epoch
    if (e_.all_observed_epoch(guard_epoch_) && e_.progress_epoch(guard_epoch_))
    {
      e_.free_epoch(dec_epoch(guard_epoch_));
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
    if (!unpinned_) {
      e_.merge_garbage(std::move(local_unlinked_), guard_epoch_);
      e_.active_[guard_epoch_]--;
      unpinned_ = true;
    }
  }

private:
  /* data */
  epoch& e_;
  unsigned guard_epoch_;
  garbage local_unlinked_;
  bool unpinned_{false};
};

} // namespace lockfree
