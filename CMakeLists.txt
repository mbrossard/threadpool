cmake_minimum_required(VERSION 3.4)
project(ThreadPool)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

find_package(Threads)

include_directories(src)

set(ParallelUtils src/threadpool.c src/threadpool.h)
add_executable(heavy tests/heavy.c ${ParallelUtils})
target_link_libraries(heavy ${CMAKE_THREAD_LIBS_INIT})

add_executable(shutdown tests/shutdown.c ${ParallelUtils})
target_link_libraries(shutdown ${CMAKE_THREAD_LIBS_INIT})

add_executable(thrdtest tests/thrdtest.c ${ParallelUtils})
target_link_libraries(shutdown ${CMAKE_THREAD_LIBS_INIT})