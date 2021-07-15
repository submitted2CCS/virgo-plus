cmake_minimum_required(VERSION 3.10)
project(VirgoPlus)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-w -march=native -faligned-new -mavx")
set(CMAKE_C_FLAGS "-w -march=native -faligned-new -mavx")

include_directories(lib)

add_definitions(-DUSE_VIRGO)
add_subdirectory(lib/virgo)

add_subdirectory(src)