cmake_minimum_required(VERSION 3.10)
project(RubiksCubeSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(cube_solver src/main.cpp src/cube.cpp)
target_include_directories(cube_solver PRIVATE include)