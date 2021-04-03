cmake_minimum_required(VERSION 3.5)

project(fontvis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fontvis src/main.cpp)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

include_directories(${CMAKE_SOURCE_DIR}/include ${GLFW_INCLUDE_DIRS})
target_link_libraries(fontvis ${GLFW_LIBRARIES} GL)
