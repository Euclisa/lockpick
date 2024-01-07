cmake_minimum_required(VERSION 3.5)

project(lockpick LANGUAGES C)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall")

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DPROJECT_DIR="${PROJECT_DIR}")

include_directories(include)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB LIB_SOURCES "${CMAKE_SOURCE_DIR}/lib/*.c")
file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.c" "${CMAKE_SOURCE_DIR}/tests/**/*.c")

file(GLOB TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/tests/")
file(GLOB LOCKPICK_INCLUDE_DIR "${CMAKE_SOURCE_DIR/include}")

add_executable(lockpick ${LIB_SOURCES} ${SOURCES})
target_include_directories(lockpick PRIVATE ${LOCKPICK_INCLUDE_DIR})


add_executable(lockpick_tests ${LIB_SOURCES} ${TEST_SOURCES})
target_include_directories(lockpick_tests PRIVATE ${TEST_INCLUDE_DIR})
target_include_directories(lockpick_tests PRIVATE ${LOCKPICK_INCLUDE_DIR})


set(UINT_CASES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests/lp_uint/cases")
set(RUN_UINT_CASES_TASK /usr/bin/python ${UINT_CASES_DIR}/generate.py --path ${UINT_CASES_DIR} 64_64 64_128 64_1024 256_256 256_1024 1024_128 1024_256 1024_1024)

add_custom_target(gen_tests COMMAND ${RUN_UINT_CASES_TASK})
add_dependencies(lockpick_tests gen_tests)