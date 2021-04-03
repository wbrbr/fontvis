cmake_minimum_required(VERSION 3.5)

project(fontvis LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(fontvis src/main.cpp)

find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include ${GLFW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(fontvis glfw OpenGL::GL ${FREETYPE_LIBRARIES})
