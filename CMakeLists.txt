cmake_minimum_required(VERSION 3.3)
project(lockfree-queue)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -stdlib=libc++ -Wall -Wextra -Weffc++ -O0")
#  -Wold-style-cast -Werror

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(benchmark benchmarks/benchmarks.cpp)

add_executable(common
    tagged_pointer.hpp
    ms_queue.hpp
    any_ptr.hpp
    atomic_ptr.hpp
    garbage.hpp
    epoch.hpp
    epoch_queue.hpp
    tests/common.cpp
)

add_executable(epoch_queue
    any_ptr.hpp
    atomic_ptr.hpp
    garbage.hpp
    epoch.hpp
    tests/epoch_queue.cpp
)

