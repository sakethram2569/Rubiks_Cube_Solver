cmake_minimum_required(VERSION 3.10)
project(RubiksCubeSolver)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

add_executable(cube_solver src/main.cpp src/cube.cpp src/search.cpp src/pdb.cpp src/bitcube.cpp src/bitcube2.cpp)
target_include_directories(cube_solver PRIVATE include)