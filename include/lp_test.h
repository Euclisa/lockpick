#ifndef _LOCKPICK_INCLUDE_LP_TEST_H
#define _LOCKPICK_INCLUDE_LP_TEST_H

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
    __LP_TEST_END
} __lp_test_actions_t;


void __lp_test_process_action(__lp_test_actions_t action, ...);


#define LP_TEST_RUN(f_call)  ({                                                         \
    struct timespec __lpt_start_ts,__lpt_end_ts;                                        \
    __lp_test_process_action(__LP_TEST_ENTER,#f_call);                                  \
    clock_gettime(CLOCK_MONOTONIC,&__lpt_start_ts);                                     \
    f_call;                                                                             \
    clock_gettime(CLOCK_MONOTONIC,&__lpt_end_ts);                                       \
    __lp_test_process_action(__LP_TEST_LEAVE,#f_call,__lpt_start_ts,__lpt_end_ts);      \
})

#define LP_TEST_ASSERT(cond, msg, ...)  ({                                              \
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


#endif // _LOCKPICK_INCLUDE_LP_TEST_H