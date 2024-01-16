file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.c" "${CMAKE_SOURCE_DIR}/tests/**/*.c")
file(GLOB TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/tests/")

add_executable(lockpick_tests ${LIB_SOURCES} ${TEST_SOURCES})
target_include_directories(lockpick_tests PRIVATE ${TEST_INCLUDE_DIR})
target_include_directories(lockpick_tests PRIVATE ${LOCKPICK_INCLUDE_DIR})

add_subdirectory(uint)
add_subdirectory(list)