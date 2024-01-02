cmake_minimum_required(VERSION 3.5)

project(lockpick LANGUAGES C)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")

include_directories(include)  

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/*.c")
file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.c" "${CMAKE_SOURCE_DIR}/tests/**/*.c")

file(GLOB TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/tests/")
file(GLOB LOCKPICK_INCLUDE_DIR "${CMAKE_SOURCE_DIR/include}")

add_executable(lockpick ${LIB_SOURCES} ${SOURCES})
add_executable(lockpick_tests ${LIB_SOURCES} ${TEST_SOURCES})

target_include_directories(lockpick_tests PRIVATE ${TEST_INCLUDE_DIR})
target_include_directories(lockpick_tests PRIVATE ${LOCKPICK_INCLUDE_DIR})

target_include_directories(lockpick PRIVATE ${LOCKPICK_INCLUDE_DIR})