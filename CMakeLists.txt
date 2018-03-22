cmake_minimum_required(VERSION 3.9)
project(parallel)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")

add_executable(parallel main.cpp File.cpp File.h)