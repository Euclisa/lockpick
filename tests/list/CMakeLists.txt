set(LIST_CASES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cases")
set(RUN_LIST_CASES_TASK /usr/bin/python ${LIST_CASES_DIR}/generate.py --path ${LIST_CASES_DIR})

add_custom_target(gen_list_tests COMMAND ${RUN_LIST_CASES_TASK})
add_dependencies(lockpick_tests gen_list_tests)