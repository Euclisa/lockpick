#ifndef _LOCKPICK_INCLUDE_TEST_H
#define _LOCKPICK_INCLUDE_TEST_H

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <time.h>


#define __LP_TEST_MAX_LEVELS 8

typedef enum __lp_test_actions
{
    __LP_TEST_INIT,
    __LP_TEST_ENTER,
    __LP_TEST_LEAVE,
    __LP_TEST_PASS,
    __LP_TEST_FAIL,
    __LP_TEST_END,
    __LP_TEST_STEP_IN,
    __LP_TEST_STEP_OUT
} __lp_test_actions_t;


void __lp_test_process_action(__lp_test_actions_t action, ...);

#define __LP_TEST_GET_OVERLOADING(_1,_2,NAME,...) NAME

#define __LP_TEST_RUN_WITH_DEPTH(f_call,max_print_depth)  ({                            \
    struct timespec __lpt_start_ts,__lpt_end_ts;                                        \
    __lp_test_process_action(__LP_TEST_ENTER,#f_call,max_print_depth);                  \
    clock_gettime(CLOCK_MONOTONIC,&__lpt_start_ts);                                     \
    f_call;                                                                             \
    clock_gettime(CLOCK_MONOTONIC,&__lpt_end_ts);                                       \
    __lp_test_process_action(__LP_TEST_LEAVE,#f_call,__lpt_start_ts,__lpt_end_ts);      \
})

#define __LP_TEST_RUN_DEFAULT(f_call) __LP_TEST_RUN_WITH_DEPTH(f_call,__LP_TEST_MAX_LEVELS)

#define LP_TEST_RUN(...) __LP_TEST_GET_OVERLOADING(__VA_ARGS__,__LP_TEST_RUN_WITH_DEPTH,__LP_TEST_RUN_DEFAULT)(__VA_ARGS__)

#define LP_TEST_STEP_INTO(f_call)   ({                                                  \
    __lp_test_process_action(__LP_TEST_STEP_IN);                                        \
    f_call;                                                                             \
    uint64_t __tests_failed;                                                            \
    __lp_test_process_action(__LP_TEST_STEP_OUT,&__tests_failed);                       \
    if(__tests_failed > 0)                                                              \
        return;                                                                         \
})

#define LP_TEST_ASSERT(cond,msg,...)  ({                                                \
    __lp_test_actions_t __lpt_asrt_act = (cond) ? __LP_TEST_PASS : __LP_TEST_FAIL;      \
    __lp_test_process_action(__lpt_asrt_act,msg,##__VA_ARGS__);                         \
    if(__lpt_asrt_act == __LP_TEST_FAIL)                                                \
        return;                                                                         \
})

#define LP_TEST_BEGIN(project_name_str)   ({                                            \
    __lp_test_process_action(__LP_TEST_INIT,project_name_str);                          \
})

#define LP_TEST_END()   ({                                                              \
    __lp_test_process_action(__LP_TEST_END);                                            \
})


#endif // _LOCKPICK_INCLUDE_TEST_H