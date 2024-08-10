find_package(Python COMPONENTS Interpreter)

set(DLIST_CASES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cases")
set(RUN_DLIST_CASES_TASK Python::Interpreter ${DLIST_CASES_DIR}/generate.py --path ${DLIST_CASES_DIR})

add_custom_target(gen_dlist_tests COMMAND ${RUN_DLIST_CASES_TASK})
add_dependencies(lockpick_unit_tests gen_dlist_tests)
add_dependencies(lockpick_stress_tests gen_dlist_tests)