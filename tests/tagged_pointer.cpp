#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "../tagged_pointer.h"


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