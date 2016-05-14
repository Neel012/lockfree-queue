#define CATCH_CONFIG_MAIN
#include <vector>
#include "../ms_queue.hpp"
#include "../epoch_queue.hpp"
#include "catch.hpp"

namespace lockfree {

struct test_counter {
  test_counter(int& i) : n{i} { n++; }

  ~test_counter() { n--; }

  int& n;
};

TEST_CASE("tagged_pointer - on stack") {
  int i = 42;

  SECTION("basic test") {
    tagged_pointer<int> tagptr(&i, 5);
    REQUIRE(tagptr.count() == 5);
    REQUIRE(tagptr.ptr() == &i);
    REQUIRE(*tagptr.ptr() == i);
  }
  SECTION("size test") {
    tagged_pointer<int> a(&i, 5);
    REQUIRE(sizeof(a) == sizeof(&i));
  }
  SECTION("operator==") {
    tagged_pointer<int> a(&i, 5);
    tagged_pointer<int> b(&i, 5);
    REQUIRE(a == b);
  }
}

TEST_CASE("tagged_pointer - on heap") {
  int* i = new int{42};

  SECTION("basic test") {
    int* i = new int{42};
    tagged_pointer<int> tagptr(i, 5);
    REQUIRE(tagptr.count() == 5);
    REQUIRE(tagptr.ptr() == i);
    REQUIRE(*tagptr.ptr() == *i);
  }
  SECTION("size test") {
    int* i = new int{42};
    tagged_pointer<int> a(i, 5);
    REQUIRE(sizeof(a) == sizeof(i));
  }
  SECTION("operator==") {
    tagged_pointer<int> a(i, 5);
    tagged_pointer<int> b(i, 5);
    REQUIRE(a == b);
  }
  delete i;
}

TEST_CASE("ms_queue - single thread") {
  ms_queue<int> q;
  q.enqueue(1);
  q.enqueue(2);
  q.enqueue(3);
  REQUIRE(*q.dequeue() == 1);
  REQUIRE(*q.dequeue() == 2);
  REQUIRE(*q.dequeue() == 3);
}

TEST_CASE("garbage_list - Basic test") {
  int counter{0};
  SECTION("Proper deallocation") {
    garbage_list<test_counter> l;
    for (int i{0}; i < 20; i++) {
      l.emplace_back(new test_counter{counter});
    }
    l.clear();
    REQUIRE(counter == 0);
  }
  SECTION("Proper deallocation 2") {
    std::vector<test_counter*> vec;
    for (int i{0}; i < 10; i++) {
      vec.push_back(new test_counter{counter});
    }
    garbage_list<test_counter> l;
    for (int i{0}; i < 10; i++) {
      l.emplace_back(vec[i]);
    }
    l.clear();
    REQUIRE(counter == 0);
  }
}

TEST_CASE("garbage - Basic test") {
  int counter{0};

  SECTION("merge one list") {
    garbage<test_counter> g;
    constexpr unsigned n = 3;
    {
      garbage_list<test_counter> l;
      for (unsigned i{0}; i < n; i++) {
        l.emplace_back(new test_counter{counter});
      }
      REQUIRE(counter == n);
      g.merge(l);
      REQUIRE(counter == n);
    }
    REQUIRE(counter == n);
  }

  SECTION("Proper deallocation") {
    garbage<test_counter> g;
    {
      garbage_list<test_counter> l;
      for (int i{0}; i < 20; i++) {
        l.emplace_back(new test_counter{counter});
      }
      garbage_list<test_counter> m;
      for (int i{0}; i < 20; i++) {
        m.emplace_back(new test_counter{counter});
      }
      g.merge(l);
      g.merge(m);
    }
    g.clear();
    REQUIRE(counter == 0);
  }
}

TEST_CASE("Epoch - Basic test") {
  int counter{0};
  {
    epoch<test_counter> e;
    std::vector<test_counter*> vec{
        new test_counter{counter}, new test_counter{counter},
        new test_counter{counter}, new test_counter{counter}};
    {
      epoch_guard<test_counter> g{e};
      for (auto& v : vec)
      {
        g.unlink(v);
      }
    }
    REQUIRE(counter == 4);
  }
  REQUIRE(counter == 0);
}

TEST_CASE("epoch_queue - single thread") {
  SECTION("3 elements") {
    epoch_queue<int> q;
    REQUIRE(q.empty());
    q.enqueue(1);
    q.enqueue(2);
    q.enqueue(3);
    REQUIRE(!q.empty());
    REQUIRE(*q.dequeue() == 1);
    REQUIRE(*q.dequeue() == 2);
    REQUIRE(*q.dequeue() == 3);
    REQUIRE(q.empty());
  }
}

} // namespace lockfree
