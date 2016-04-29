cmake_minimum_required(VERSION 3.4)
project(lockfree-queue)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(epoch_queue
    queue.h
    epoch_queue.h
    tests/run_epoch_tests.cpp
)

add_executable(ms_queue
    queue.h
    ms_queue.h
    tests/run_ms_tests.cpp
)
