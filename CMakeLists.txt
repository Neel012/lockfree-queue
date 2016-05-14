cmake_minimum_required(VERSION 3.3)
project(lockfree-queue)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -stdlib=libc++ -Wall -Wextra -Weffc++ -Wold-style-cast -Werror -O0")

add_executable(enqueue_speed benchmarks/enqueue_speed.cpp)