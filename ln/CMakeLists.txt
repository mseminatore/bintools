cmake_minimum_required(VERSION 3.20)

project(ln VERSION 1.0)

include_directories(${PROJECT_SOURCE_DIR})

add_executable(ln main.cpp ../aout.cpp)
