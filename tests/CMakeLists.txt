file(GLOB TEST_SOURCES "${CMAKE_SOURCE_DIR}/tests/*.c" "${CMAKE_SOURCE_DIR}/tests/**/*.c")
file(GLOB TEST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/tests/")

set(UNIT_TEST_FLAGS -fsanitize=address -fsanitize=undefined)
set(STRESS_TEST_FLAGS -O3)

add_executable(lockpick_unit_tests ${LIB_SOURCES} ${TEST_SOURCES})
target_include_directories(lockpick_unit_tests PRIVATE ${TEST_INCLUDE_DIR} ${LOCKPICK_INCLUDE_DIR})
target_compile_options(lockpick_unit_tests PRIVATE ${UNIT_TEST_FLAGS})
target_compile_definitions(lockpick_unit_tests PUBLIC LOCKPICK_DEBUG)
target_link_libraries(lockpick_unit_tests ${UNIT_TEST_FLAGS})

add_executable(lockpick_stress_tests ${LIB_SOURCES} ${TEST_SOURCES})
target_include_directories(lockpick_stress_tests PRIVATE ${TEST_INCLUDE_DIR} ${LOCKPICK_INCLUDE_DIR})
target_compile_options(lockpick_stress_tests PRIVATE ${STRESS_TEST_FLAGS})

add_subdirectory(uint)
add_subdirectory(list)