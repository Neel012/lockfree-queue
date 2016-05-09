#include <atomic>
#include <memory>
#include "tests/catch.hpp"

namespace lockfree {

// fixme: rename to ?garbage
template <typename T>
struct list {
  struct node {
    T data;
    //std::unique_ptr<node> next;
    node* next = nullptr;
  };

  // Assumes rhs is not beeing accessed concurently
  // Precondition: Epochs do not observe progress for the duration of this operation.
  void merge(list& rhs) {
    if (rhs.head_.load() == nullptr) {
      return;
    }
    while (true) {
      node* head = head_.load();
      rhs.tail_->next = head;
      if (head_.compare_exchange_weak(head, rhs.head_)) {
        break;
      }
    }
  }

  // template <typename... Args>
  // void emlpace_front(Args&&... args) {
  //  //auto new_node = std::make_unique<node>(std::forward<Args>(args)..., head_.load());
  //  head_.store(new_node);
  //  if (tail_ == nullptr) {
  //    tail_ = new_node;
  //  }
  //}

  // TODO: make sure unique_ptr takes ownership of passed pointer
  template <typename... Args>
  void emlpace_back(Args&&... args) {
    auto new_node = new node{std::forward<Args>(args)...};
    if (tail_ == nullptr) {
      tail_ = new_node;
      head_.store(new_node);
    } else {
      tail_->next = new_node;
    }
  }

  void clear() {
    node* n = head_.load();
    if (n == nullptr) {
      return;
    }
    while (n->next != nullptr) {
      node* d = n;
      n = n->next;
      delete d;
    }
  }

private:
  std::atomic<node*> head_ {nullptr};
  node* tail_ {nullptr};
};

namespace {

struct test_counter {
  static int n;

  test_counter() { n++; }

  ~test_counter() { n--; }
};

int test_counter::n{0};

}

TEST_CASE("Garbage List - Basic test") {
  SECTION("Proper deallocation") {
    list<test_counter> l;
    for (int i{0}; i < 20; i++) {
      l.emlpace_back();
    }
    l.clear();
    REQUIRE(test_counter::n == 0);
  }
}

}
