#include <lockpick/test.h>
#include <lockpick/affirmf.h>
#include <lockpick/utility.h>
#include <lockpick/sync/spinlock_bitset.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>



lp_spinlock_bitset_t *spins;
size_t *counters;
const size_t max_count = 1000;

void *__count(void *args)
{
    size_t counter_i = *((size_t*)args);
    bool *status = (bool*)malloc(sizeof(bool));
    *status = false;
    for(size_t i = 0; i < max_count; ++i)
    {
        if(!lp_spinlock_bitset_lock(spins,counter_i))
            return status;
        ++counters[counter_i];
        if(!lp_spinlock_bitset_unlock(spins,counter_i))
            return status;
    }
    *status = true;
    return status;
}


void test_spinlock_bitset_multiple_counters(size_t counters_num, size_t threads_per_counter)
{
    counters = (size_t*)calloc(counters_num,sizeof(size_t));
    spins = lp_spinlock_bitset_init(counters_num);

    const size_t threads_total = counters_num*threads_per_counter;
    size_t *counters_exec_order = (size_t*)malloc(threads_total*sizeof(size_t));
    for(size_t i = 0; i < threads_per_counter; ++i)
        for(size_t counter_i = 0; counter_i < counters_num; ++counter_i)
            counters_exec_order[i*counters_num+counter_i] = counter_i;
    
    lp_shuffle(counters_exec_order,threads_total);

    pthread_t threads[threads_total];
    for(size_t thr_i = 0; thr_i < threads_total; ++thr_i)
        pthread_create(&threads[thr_i],NULL,__count,&counters_exec_order[thr_i]);

    for(size_t thr_i = 0; thr_i < threads_total; ++thr_i)
    {
        bool *status;
        pthread_join(threads[thr_i],(void**)&status);
        affirmf(*status,"Something went wrong inside thread %ld",thr_i);
        free(status);
    }

    for(size_t counter_i = 0; counter_i < counters_num; ++counter_i)
    {
        size_t expected = max_count*threads_per_counter;
        LP_TEST_ASSERT(counters[counter_i] == expected,
            "Expected: %ld, got: %ld for counter %ld",expected,counters[counter_i],counter_i);
    }

    lp_test_cleanup:
    lp_spinlock_bitset_release(spins);
    free(counters);
    free(counters_exec_order);
}


void lp_test_spinlock_bitset()
{
    LP_TEST_RUN(test_spinlock_bitset_multiple_counters(10,10));
    LP_TEST_RUN(test_spinlock_bitset_multiple_counters(100,10));
    LP_TEST_RUN(test_spinlock_bitset_multiple_counters(10,100));
    LP_TEST_RUN(test_spinlock_bitset_multiple_counters(100,100));
}