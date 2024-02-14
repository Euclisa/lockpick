#include <lockpick/test.h>
#include <lockpick/sync/lock_graph.h>
#include <lockpick/sync/bits.h>
#include <lockpick/utility.h>
#include <lockpick/affirmf.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>


void __lp_build_random_graph_and_masks(lp_lock_graph_t *lgraph, uint32_t *lmasks, uint32_t deps_num)
{
    uint32_t blocks_num = lp_lock_graph_blocks_num(lgraph);
    uint32_t set_deps_remain = deps_num;
    uint32_t total_deps_remain = blocks_num*blocks_num-((blocks_num*(blocks_num-1))/2);

    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
    {
        for(uint32_t lockee_i = block_i; lockee_i < blocks_num; ++lockee_i)
        {
            double set_dep_prob = (double)set_deps_remain / (double)total_deps_remain;
            double gen = ((double)rand() / (double)RAND_MAX);
            bool set_lock = gen <= set_dep_prob;
            if(set_lock)
            {
                lmasks[lockee_i] |= 1 << block_i;
                affirmf(lp_lock_graph_add_dep_mutual(lgraph,block_i,lockee_i),
                    "Failed to add dependency: %d -> %d",block_i,lockee_i);
                --set_deps_remain;
            }
            --total_deps_remain;
        }
    }
    
    lp_lock_graph_commit(lgraph);
}


volatile _Atomic uint32_t exec_mask = 0;
uint32_t *lmasks;
lp_lock_graph_t *lgraph;


void *__block(void *args)
{
    uint32_t block_i = *(uint32_t*)args;
    return_on(!lp_lock_graph_lock(lgraph,block_i),NULL);

    uint32_t *curr_exec_mask = (uint32_t*)malloc(sizeof(uint32_t));
    *curr_exec_mask = exec_mask;
    exec_mask |= 1 << block_i;

    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 100;
    nanosleep(&t,NULL);

    exec_mask &= ~(1 << block_i);
    *curr_exec_mask |= exec_mask;

    return_on(!lp_lock_graph_unlock(lgraph,block_i),NULL);

    return curr_exec_mask;
}


void __test_lock_graph_random_graph(uint32_t blocks_num, uint32_t deps_num)
{
    lgraph = lp_lock_graph_create(blocks_num);
    lmasks = (uint32_t*)calloc(blocks_num,sizeof(uint32_t));
    __lp_build_random_graph_and_masks(lgraph,lmasks,deps_num);

    const uint32_t threads_per_block = 20;
    const uint32_t threads_total = blocks_num*threads_per_block;
    uint32_t *blocks_exec_order = (uint32_t*)malloc(threads_total*sizeof(uint32_t));
    for(uint32_t i = 0; i < threads_per_block; ++i)
        for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
            blocks_exec_order[i*blocks_num+block_i] = block_i;
    
    lp_shuffle(blocks_exec_order,threads_total);

    pthread_t threads[threads_total];
    for(uint32_t thr_i = 0; thr_i < threads_total; ++thr_i)
        pthread_create(&threads[thr_i],NULL,__block,&blocks_exec_order[thr_i]);
    
    bool test_status = true;
    uint32_t fail_thread_i = 0;
    uint32_t fail_block_i = 0;
    uint32_t fail_exec_mask = 0;
    uint32_t fail_lmask = 0;
    for(size_t thr_i = 0; thr_i < threads_total; ++thr_i)
    {
        uint32_t *block_exec_mask;
        uint32_t curr_block_i = blocks_exec_order[thr_i];
        uint32_t curr_lmask = lmasks[curr_block_i];
        pthread_join(threads[thr_i],(void**)&block_exec_mask);
        if(test_status && (*block_exec_mask & curr_lmask))
        {
            fail_thread_i = thr_i;
            fail_block_i = curr_block_i;
            fail_exec_mask = *block_exec_mask; 
            fail_lmask = curr_lmask;
            test_status = false;
        }
        free(block_exec_mask);
    }
    
    LP_TEST_ASSERT(test_status,
        "Lock hierarchy inconsistency in thread %d, block %d; block exec mask: %d, block lmask: %d",
        fail_thread_i,fail_block_i,fail_exec_mask,fail_lmask);

    lp_test_cleanup:
    lp_lock_graph_release(lgraph);
    free(lmasks);
    free(blocks_exec_order);
}


void test_lock_graph_random_graph(uint32_t blocks_num, uint32_t deps_num)
{
    const uint32_t tests_num = 15;
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
        LP_TEST_STEP_INTO(__test_lock_graph_random_graph(blocks_num,deps_num));
    lp_test_cleanup:
}


void lp_test_lock_graph()
{
    srand(0);
    LP_TEST_RUN(test_lock_graph_random_graph(2,0));
    LP_TEST_RUN(test_lock_graph_random_graph(2,2));
    LP_TEST_RUN(test_lock_graph_random_graph(2,4));
    
    LP_TEST_RUN(test_lock_graph_random_graph(4,0));
    LP_TEST_RUN(test_lock_graph_random_graph(4,4));
    LP_TEST_RUN(test_lock_graph_random_graph(4,8));
    LP_TEST_RUN(test_lock_graph_random_graph(4,12));
    LP_TEST_RUN(test_lock_graph_random_graph(4,16));

    LP_TEST_RUN(test_lock_graph_random_graph(10,0));
    LP_TEST_RUN(test_lock_graph_random_graph(10,10));
    LP_TEST_RUN(test_lock_graph_random_graph(10,20));
    LP_TEST_RUN(test_lock_graph_random_graph(10,30));
    LP_TEST_RUN(test_lock_graph_random_graph(10,40));
    LP_TEST_RUN(test_lock_graph_random_graph(10,50));
    LP_TEST_RUN(test_lock_graph_random_graph(10,60));
    LP_TEST_RUN(test_lock_graph_random_graph(10,70));
    LP_TEST_RUN(test_lock_graph_random_graph(10,80));
    LP_TEST_RUN(test_lock_graph_random_graph(10,90));
    LP_TEST_RUN(test_lock_graph_random_graph(10,100));
}