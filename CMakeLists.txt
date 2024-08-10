cmake_minimum_required(VERSION 3.5)

project(lockpick LANGUAGES C)

set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11 -Wall -g")

set(LOCKPICK_PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DLOCKPICK_PROJECT_DIR="${LOCKPICK_PROJECT_DIR}" -D_GNU_SOURCE -DUNW_LOCAL_ONLY -DCL_TARGET_OPENCL_VERSION=300)

include_directories(include)

file(GLOB SOURCES CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/lockpick/graph/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/graph/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/graph/properties/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/graph/traverse/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/graph/tsort/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/inference/inference_graph/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/inference/host/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/inference/ocl/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/functions/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/graph/types/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/slab/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/sync/*.c"
        "${CMAKE_SOURCE_DIR}/lockpick/ocl/*.c")
file(GLOB ARCH_SOURCES CMAKE_CONFIGURE_DEPENDS
        "${CMAKE_SOURCE_DIR}/arch/x86_64/*.c"
        "${CMAKE_SOURCE_DIR}/arch/x86_64/sync/*.c")

find_library(UNWIND_LIB REQUIRED NAMES unwind)
find_package(OpenCL REQUIRED)


file(GLOB LOCKPICK_INCLUDE_DIR "${CMAKE_SOURCE_DIR/include}")

add_executable(lockpick "lockpick/entry-points/main.c" ${SOURCES} ${ARCH_SOURCES})
target_link_libraries(lockpick ${UNWIND_LIB} OpenCL::OpenCL)
target_include_directories(lockpick PRIVATE ${LOCKPICK_INCLUDE_DIR})

# Must set NEOReadDebugKeys=1 and DisableDeepBind=1 to run with sanitizer
add_executable(lockpick_ocl_fetch "lockpick/entry-points/ocl_fetch.c" ${SOURCES} ${ARCH_SOURCES})
target_link_libraries(lockpick_ocl_fetch ${UNWIND_LIB} OpenCL::OpenCL)
#target_compile_options(lockpick_ocl_fetch PRIVATE -fsanitize=address,undefined)
#target_link_options(lockpick_ocl_fetch PRIVATE -fsanitize=address,undefined)
target_include_directories(lockpick_ocl_fetch PRIVATE ${LOCKPICK_INCLUDE_DIR})

add_subdirectory(tests)