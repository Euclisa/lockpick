#include <lockpick/affinity.h>
#include <lockpick/affirmf.h>
#include <pthread.h>
#include <malloc.h>


uint32_t lp_affinity_cpu_count;
cpu_set_t *lp_affinity_cpus;


void __lp_affinity_init()
{
    cpu_set_t *main_aff;
    affirmf(!sched_getaffinity(0,sizeof(cpu_set_t),main_aff),"Failed to get affinity for main thread");
    lp_affinity_cpu_count = CPU_COUNT(main_aff);

    size_t lp_affinity_cpus_size = lp_affinity_cpu_count*sizeof(cpu_set_t);
    lp_affinity_cpus = (cpu_set_t*)malloc(lp_affinity_cpus_size);
    affirm_bad_malloc(lp_affinity_cpus,"available cpus affinity masks array",lp_affinity_cpus_size);
    
    uint32_t aff_cpu_i = 0;
    uint32_t cpu_i = 0;
    for(uint32_t cpu_i = 0; aff_cpu_i < lp_affinity_cpu_count; ++cpu_i)
    {
        if(CPU_ISSET(cpu_i,main_aff))
        {
            cpu_set_t *aff = lp_affinity_cpus+aff_cpu_i;
            CPU_ZERO(aff);
            CPU_SET(cpu_i,aff);
            ++aff_cpu_i;
        }
    }
}