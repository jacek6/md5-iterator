cmake_minimum_required(VERSION 3.0)

project(proj2)

add_executable(proj2 main.cpp)

install(TARGETS proj2 RUNTIME DESTINATION bin)
