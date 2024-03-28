#ifndef _LOCKPICK_AFFINITY
#define _LOCKPICK_AFFINITY

#ifndef _GNU_SOURCE
#error "lockpick/affinity.h requires _GNU_SOURCE to be defined"
#endif // _GNU_SOURCE

#include <stdint.h>
#include <sched.h>


extern uint32_t lp_affinity_cpu_count;
extern cpu_set_t *lp_affinity_cpus;


void __lp_affinity_init();

#endif // _LOCKPICK_AFFINITY