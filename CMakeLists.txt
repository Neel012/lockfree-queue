cmake_minimum_required(VERSION 3.4)
project(lockfree-queue)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(common
    tagged_pointer.hpp
    ms_queue.hpp
    garbage_list.hpp
    epoch.hpp
    epoch_queue.hpp
    tests/common.cpp
)

add_executable(epoch
    garbage_list.hpp
    epoch.hpp
    tests/epoch.cpp
)

add_executable(tagged_pointer
    tagged_pointer.hpp
    tests/tagged_pointer.cpp
)
