cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/pybind11)

# python package ----------------------------------------------
project(VectorLoop)

pybind11_add_module(VectorLoop pybind.cpp)
target_include_directories(VectorLoop PUBLIC
        submodules/pybind11/include
        /usr/include/python3.10)
