cmake_minimum_required(VERSION 3.17)
project(RayTracingBase)

set(CMAKE_CXX_STANDARD 17)

ADD_SUBDIRECTORY(Libs/glm)
add_executable(RayTracingBase main.cpp)