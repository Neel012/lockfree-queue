cmake_minimum_required(VERSION 3.3)
project(lockfree-queue)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -stdlib=libc++ -Wall -Wextra -Weffc++ -O0")
#  -Wold-style-cast -Werror

add_executable(enqueue_speed benchmarks/enqueue_speed.cpp)
