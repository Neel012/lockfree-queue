#include "../any_ptr.hpp"
#include "../atomic_ptr.hpp"
#include "../epoch.hpp"
#include "../garbage.hpp"
#include "../tagged_pointer.hpp"
#include "catch.hpp"
#include <vector>

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

TEST_CASE("any_ptr - Basic test") {
  int counter{0};
  auto* t = new test_counter{counter};
  SECTION("Correct destruction") {
    {
      any_ptr a{t};
      REQUIRE(counter == 1);
    }
    REQUIRE(counter == 0);
  }
  SECTION("Move constructor") {
    {
      //auto a = make_any_ptr(t);
      any_ptr a{t};
      //REQUIRE(counter == 1);
      any_ptr b{std::move(a)};
      REQUIRE(counter == 1);
    }
    REQUIRE(counter == 0);
  }
}

TEST_CASE("any_ptr - vector") {
  int counter{0};
  SECTION("dtor") {
    {
      std::vector<any_ptr> data;
      for (int i{0}; i < 20; i++) {
        data.emplace_back(new test_counter{counter});
      }
      REQUIRE(counter == 20);
    }
    REQUIRE(counter == 0);
  }
  SECTION("move") {
    {
      std::vector<any_ptr> data;
      for (int i{0}; i < 20; i++) {
        data.emplace_back(new test_counter{counter});
      }
      REQUIRE(counter == 20);
      std::vector<any_ptr> m{std::move(data)};
      REQUIRE(counter == 20);
      data.clear();
      REQUIRE(counter == 20);
    }
    REQUIRE(counter == 0);
  }
}

TEST_CASE("garbage - Basic test") {
  int counter{0};
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

} // namespace lockfree
