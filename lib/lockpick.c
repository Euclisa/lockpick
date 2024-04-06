#include <lockpick/lockpick.h>
#include <lockpick/affirmf.h>
#include <stdio.h>


bool lp_initialized = false;

lp_logger_t *lp_log;


void lp_init()
{
    // Can't call affirmf because it might be uninitialized
    if(lp_initialized)
        errorf("Multiple call for 'lp_init' is not allowed");

    __lp_affirmf_init();
    __lp_exit_init();
    __lp_affinity_init();

    lp_log = lp_logger_create("[%H:%M:%s] %L: %u");

    lp_initialized = true;

    lp_logger_debug(lp_log,"Initialization successful!");
}