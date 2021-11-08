cmake_minimum_required(VERSION 3.20)
project(Freecam VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)
add_library(Freecam SHARED Freecam.cpp mem.cpp proc.cpp)

add_subdirectory(minhook)
target_include_directories(Freecam PRIVATE "${CMAKE_CURRENT_LIST_DIR}/minhook/include")
target_link_libraries(Freecam PRIVATE minhook)