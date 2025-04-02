cmake_minimum_required(VERSION 3.16)

project(ThreadPool LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ThreadPool SHARED
  ThreadPool_global.h
  threadpool.cpp
  threadpool.h
)


add_executable(Test
    test.cpp
)

target_link_libraries(Test
    ThreadPool
)

target_compile_definitions(ThreadPool PRIVATE THREADPOOL_LIBRARY)
