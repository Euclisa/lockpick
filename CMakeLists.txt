cmake_minimum_required(VERSION 3.5)

project(lockpick LANGUAGES C)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -w")

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DPROJECT_DIR="${PROJECT_DIR}")

include_directories(include)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/*.c")

file(GLOB LOCKPICK_INCLUDE_DIR "${CMAKE_SOURCE_DIR/include}")

add_executable(lockpick ${LIB_SOURCES} ${SOURCES})
target_include_directories(lockpick PRIVATE ${LOCKPICK_INCLUDE_DIR})

add_subdirectory(tests)