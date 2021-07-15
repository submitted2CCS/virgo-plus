cmake_minimum_required(VERSION 3.10)
project(virgo)
set(CMAKE_CXX_STANDARD 14)

include_directories(src)
link_directories(src)

add_subdirectory(src)
