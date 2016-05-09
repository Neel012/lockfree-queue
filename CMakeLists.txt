cmake_minimum_required(VERSION 3.4)
project(lockfree-queue)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
