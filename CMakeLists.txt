cmake_minimum_required(VERSION 3.28)

project(RayTracer)

add_executable(RayTracer main.cpp)
target_include_directories(RayTracer PRIVATE .)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


