#include <array>
#include <thread>
#include <vector>

#include <tests/catch.hpp>
#include <atomic_ptr.hpp>
#include <epoch.hpp>
#include <garbage.hpp>
#include <tagged_pointer.hpp>

namespace lockfree {

struct test_counter {
  test_counter(int& i) : n{i} { n++; }

  ~test_counter() { n--; }

  int& n;
};

struct test_atomic_counter {
  test_atomic_counter(std::atomic<int>* i) : n{i} { *n++; }

  ~test_atomic_counter() { *n--; }

  std::atomic<int>* n;
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

TEST_CASE("atomic_ptr - operator=") {
  int counter{0};
  SECTION("move") {
    {
      atomic_ptr<test_counter> p{new test_counter{counter}};
      atomic_ptr<test_counter> i{std::move(p)};
    }
    REQUIRE(counter == 0);
  }
}

TEST_CASE("garbage - Basic test") {
  int counter{0};
  SECTION("move operator=") {
    {
      garbage g;
      g.emplace_back(new test_counter{counter});
      garbage h;
      h = std::move(g);
    }
    REQUIRE(counter == 0);
  }
  SECTION("One garbage clear") {
    garbage l;
    for (int i{0}; i < 20; i++) {
      l.emplace_back(new test_counter{counter});
    }
    l.clear();
    REQUIRE(counter == 0);
  }
  SECTION("One garbage clear II") {
    std::vector<test_counter*> vec;
    for (int i{0}; i < 10; i++) {
      vec.push_back(new test_counter{counter});
    }
    REQUIRE(counter == 10);
    garbage l;
    for (int i{0}; i < 10; i++) {
      l.emplace_back(vec[i]);
    }
    l.clear();
    REQUIRE(counter == 0);
  }
  SECTION("garbage dtor") {
    {
      garbage g;
      g.emplace_back(new test_counter{counter});
    }
    REQUIRE(counter == 0);
  }
  SECTION("garbage move dtor") {
    {
      garbage h;
      {
        garbage g;
        g.emplace_back(new test_counter{counter});
        h = std::move(g);
      }
      REQUIRE(counter == 1);
    }
    REQUIRE(counter == 0);
  }
}

TEST_CASE("garbage_stack - Basic test") {
  int counter{0};

  SECTION("merge one list") {
    garbage_stack g;
    constexpr unsigned n = 3;
    {
      garbage l;
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
    garbage_stack g;
    {
      garbage l;
      for (int i{0}; i < 20; i++) {
        l.emplace_back(new test_counter{counter});
      }
      garbage m;
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

void test_garbage_stack(size_t stack_count) {
  int counter{0};
  garbage_stack g;
  {
    size_t sum{0};
    for (size_t i{0}; i < stack_count; ++i) {
      REQUIRE(counter == sum);
      garbage l;
      for (int j{0}; j < 5; j++) {
        l.emplace_back(new test_counter{counter});
        sum++;
      }
      g.merge(l);
    }
  }
  // REQUIRE(g.count_nodes() == stack_count);
  g.clear();
  REQUIRE(counter == 0);
}

TEST_CASE("garbage_stack - Big stack") {
  SECTION("40 000 nodes") {
    test_garbage_stack(40000);
  }
  SECTION("60 000 nodes") {
    test_garbage_stack(60'000);
  }
  SECTION("140 000 nodes") {
    test_garbage_stack(140'000);
  }
}

TEST_CASE("Epoch - Basic test") {
  int counter{0};
  {
    epoch e;
    std::vector<test_counter*> vec{
        new test_counter{counter}, new test_counter{counter},
        new test_counter{counter}, new test_counter{counter}};
    {
      epoch_guard g{e};
      for (auto& v : vec)
      {
        g.unlink(v);
      }
    }
    REQUIRE(counter == 4);
  }
  REQUIRE(counter == 0);
}

TEST_CASE("Epoch - guard") {
  std::atomic<int> counter{0};
  constexpr unsigned COUNT{10};
  std::array<std::thread, COUNT> threads;
  epoch e;
  for (auto& t : threads) {
    t = std::thread([&] {
      epoch_guard g{e};
      g.unlink(new test_atomic_counter{&counter});
    });
  }
  for (auto& t : threads) {
    t.join();
  }
  for (unsigned i = 0; i < 3; i++) {
    epoch_guard g{e};
  }
  REQUIRE(counter == 0);
}

} // namespace lockfree
