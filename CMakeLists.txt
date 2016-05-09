cmake_minimum_required(VERSION 3.3)
project(lockfree-queue)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -stdlib=libc++ -Wall -Wextra -Weffc++ -O0")
#  -Wold-style-cast -Werror

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(enqueue_speed benchmarks/benchmarks.cpp)

add_executable(common
    tagged_pointer.h
    ms_queue.h
    list.h
    epoch.h
    epoch_queue.h
    tests/common.cpp
)

add_executable(epoch
    list.h
    epoch.h
    tests/epoch.cpp
)

add_executable(tagged_pointer
    tagged_pointer.h
    tests/tagged_pointer.cpp
)
