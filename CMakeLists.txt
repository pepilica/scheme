cmake_minimum_required(VERSION 3.17)
project(scheme_interpreter)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -Wall")


include(sources.cmake)

add_executable(scheme_interpreter repl/main.cpp)
target_link_libraries(scheme_interpreter scheme_libs)
