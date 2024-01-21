set(UINT_CASES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cases")
set(RUN_UINT_CASES_TASK /usr/bin/python ${UINT_CASES_DIR}/generate.py --path ${UINT_CASES_DIR} 64_64 64_128 64_1024 256_256 256_1024 1024_128 1024_256 1024_1024 64 256 1024)

add_custom_target(gen_uint_tests COMMAND ${RUN_UINT_CASES_TASK})
add_dependencies(lockpick_unit_tests gen_uint_tests)
add_dependencies(lockpick_stress_tests gen_uint_tests)