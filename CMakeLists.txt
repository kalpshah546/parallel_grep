cmake_minimum_required(VERSION 3.10)

project(parallel_grep LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable optimizations for a "release-style" build by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O2")

find_package(Threads REQUIRED)

add_executable(parallel_grep
    main.cpp
    searcher.cpp
    benchmark.cpp
    threadpool.cpp
)

add_executable(threadpool_test
    threadpool_test.cpp
    threadpool.cpp
)

target_link_libraries(parallel_grep PRIVATE Threads::Threads)
target_link_libraries(threadpool_test PRIVATE Threads::Threads)