cmake_minimum_required(VERSION 3.3)
project(lockfree-queue)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}
)

add_executable(benchmark
  ms_queue.hpp
  epoch_queue.hpp
  my_queue.hpp
  mutex_queue.hpp
  benchmarks/benchmarks.cpp
)

add_executable(common
    tests/main.cpp
    tests/common.cpp
    tagged_pointer.hpp
    atomic_ptr.hpp
    garbage.hpp
    epoch.hpp
)

add_executable(ms_queue
    tests/main.cpp
    tagged_pointer.hpp
    ms_queue.hpp
    tests/queue_tests.hpp
    tests/ms_queue.cpp
)

add_executable(epoch_queue
    tests/main.cpp
    atomic_ptr.hpp
    garbage.hpp
    epoch.hpp
    epoch_queue.hpp
    tests/queue_tests.hpp
    tests/epoch_queue.cpp
)

add_executable(my_queue
    tests/main.cpp
    tests/queue_tests.hpp
    tests/my_queue.cpp
)
 
target_link_libraries(benchmark ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(common ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(ms_queue ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(epoch_queue ${CMAKE_THREAD_LIBS_INIT} )
target_link_libraries(my_queue ${CMAKE_THREAD_LIBS_INIT} )

