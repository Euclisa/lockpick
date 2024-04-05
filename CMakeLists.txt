cmake_minimum_required(VERSION 3.5)

project(lockpick LANGUAGES C)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -g")

set(LOCKPICK_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DLOCKPICK_PROJECT_DIR="${LOCKPICK_PROJECT_DIR}" -D_GNU_SOURCE -DUNW_LOCAL_ONLY -DCL_TARGET_OPENCL_VERSION=300)

include_directories(include)

file(GLOB SOURCES CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/src/graph/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/graph/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/graph/properties/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/graph/traverse/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/graph/tsort/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/inference/inference_graph/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/inference/host/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/functions/*.c"
        "${CMAKE_SOURCE_DIR}/src/graph/types/*.c")
file(GLOB LIB_SOURCES CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/lib/*.c"
        "${CMAKE_SOURCE_DIR}/lib/sync/*.c")
file(GLOB ARCH_SOURCES CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/arch/x86_64/*.c"
        "${CMAKE_SOURCE_DIR}/arch/x86_64/sync/*.c")

find_library(UNWIND_LIB REQUIRED NAMES unwind)

file(GLOB LOCKPICK_INCLUDE_DIR "${CMAKE_SOURCE_DIR/include}")

add_executable(lockpick "src/main.c" ${LIB_SOURCES} ${SOURCES} ${ARCH_SOURCES})
target_link_libraries(lockpick ${UNWIND_LIB})
target_include_directories(lockpick PRIVATE ${LOCKPICK_INCLUDE_DIR})

add_subdirectory(tests)