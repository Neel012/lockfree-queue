#include <atomic>
#include <memory>

namespace lockfree {

// fixme: rename to ?garbage
template <typename T>
struct list {
  struct node {
    T data;
    //std::unique_ptr<node> next;
    node* next;
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

  template <typename... Args>
  void emlpace_back(Args&&... args) {
    //auto new_node = std::make_unique<node>(std::forward<Args>(args)..., nullptr);
    auto new_node = new node{std::forward<Args>(args)..., nullptr};
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

}
