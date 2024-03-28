#include <lockpick/lockpick.h>


void lp_init()
{
    __lp_affirmf_init();
    __lp_exit_init();
    __lp_affinity_init();
}