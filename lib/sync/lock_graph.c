#include <lockpick/sync/lock_graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <lockpick/emalloc.h>
#include <stdlib.h>


lp_lock_graph_t *lp_lock_graph_create(uint32_t blocks_num)
{
    size_t graph_size = sizeof(lp_lock_graph_t);
    lp_lock_graph_t *graph = (lp_lock_graph_t*)malloc(sizeof(lp_lock_graph_t));
    affirm_bad_malloc(graph,"lock graph",graph_size);

    size_t lockee_list_size = blocks_num*sizeof(uint32_t*);
    graph->__lockee_list = (uint32_t**)malloc(lockee_list_size);
    affirm_bad_malloc(graph->__lockee_list,"lockee list",lockee_list_size);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
    {
        graph->__lockee_list[block_i] = (uint32_t*)malloc(0);
        affirm_bad_malloc(graph->__lockee_list[block_i],"block lockee list",0);
    }

    graph->__lockee_list_sizes = (uint32_t*)calloc(blocks_num,sizeof(uint32_t));
    affirm_bad_malloc(graph->__lockee_list_sizes,"lockee list sizes list",blocks_num*sizeof(uint32_t));

    graph->__lock_counters = (uint32_t*)calloc(blocks_num,sizeof(uint32_t));
    affirm_bad_malloc(graph->__lock_counters,"lock counters",blocks_num*sizeof(uint32_t));

    size_t block_conds_size = blocks_num*sizeof(pthread_cond_t);
    graph->__block_conds = (pthread_cond_t*)malloc(block_conds_size);
    affirm_bad_malloc(graph->__block_conds,"block conditions",block_conds_size);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
    {
        uint32_t s = pthread_cond_init(&graph->__block_conds[block_i],NULL);
        affirmf(s == 0,"Failed to init condition for block %d, returned status: %d",block_i,s);
    }

    uint32_t s = pthread_mutex_init(&graph->__counters_lock,NULL);
    affirmf(s == 0,"Failed to init graph mutex, returned status: %d",s);

    graph->blocks_num = blocks_num;
    graph->commited = false;

    return graph;
}


void lp_lock_graph_release(lp_lock_graph_t *graph)
{
    affirm_nullptr(graph,"lock graph");

    for(uint32_t block_i = 0; block_i < graph->blocks_num; ++block_i)
    {
        free(graph->__lockee_list[block_i]);
        pthread_cond_destroy(&graph->__block_conds[block_i]);
    }

    pthread_mutex_destroy(&graph->__counters_lock);

    free(graph->__block_conds);
    free(graph->__lockee_list);
    free(graph->__lockee_list_sizes);
    free(graph->__lock_counters);
    free(graph);
}


static inline void __lp_lock_graph_add_dep(lp_lock_graph_t *graph, uint32_t locker, uint32_t lockee)
{
    uint32_t curr_locker_list_size = graph->__lockee_list_sizes[locker];
    for(uint32_t lockee_i = 0; lockee_i < curr_locker_list_size; ++lockee_i)
        affirmf(graph->__lockee_list[locker][lockee_i] != lockee,
            "Given lockee %d is already locked by specified %d block",lockee,locker);

    size_t lockee_list_new_size = (curr_locker_list_size+1)*sizeof(uint32_t);
    graph->__lockee_list[locker] = (uint32_t*)realloc(graph->__lockee_list[locker],lockee_list_new_size);
    affirm_bad_malloc(graph->__lockee_list[locker],"block lockee list",lockee_list_new_size);
    graph->__lockee_list[locker][curr_locker_list_size] = lockee;
    ++graph->__lockee_list_sizes[locker];
}


void lp_lock_graph_add_dep(lp_lock_graph_t *graph, uint32_t locker, uint32_t lockee)
{
    affirm_nullptr(graph,"lock graph");
    affirmf(locker < graph->blocks_num,"Locker index %d is out of range (max: %d)",locker,graph->blocks_num-1);
    affirmf(lockee < graph->blocks_num,"Lockee index %d is out of range (max: %d)",lockee,graph->blocks_num-1);
    affirmf(!graph->commited,"Specified graph is commited");

    __lp_lock_graph_add_dep(graph,locker,lockee);
}


void lp_lock_graph_add_dep_mutual(lp_lock_graph_t *graph, uint32_t a, uint32_t b)
{
    affirm_nullptr(graph,"lock graph");
    affirmf(a < graph->blocks_num,"Block index %d is out of range (max: %d)",a,graph->blocks_num-1);
    affirmf(b < graph->blocks_num,"Block index %d is out of range (max: %d)",b,graph->blocks_num-1);
    affirmf(!graph->commited,"Specified graph is commited");
    
    __lp_lock_graph_add_dep(graph,a,b);
    if(a != b)
        __lp_lock_graph_add_dep(graph,b,a);
}


inline void lp_lock_graph_commit(lp_lock_graph_t *graph)
{
    affirm_nullptr(graph,"lock graph");
    affirmf(!graph->commited,"Specified graph is commited");

    graph->commited = true; 
}


void lp_lock_graph_lock(lp_lock_graph_t *graph, uint32_t block_i)
{
    affirm_nullptr(graph,"lock graph");
    affirmf(block_i < graph->blocks_num,"Block index %d is out of range (max: %d)",block_i,graph->blocks_num-1);
    affirmf(graph->commited,"Specified graph must be commited");

    pthread_mutex_lock(&graph->__counters_lock);

    while(graph->__lock_counters[block_i] != 0)
        pthread_cond_wait(&graph->__block_conds[block_i],&graph->__counters_lock);

    for(uint32_t lockee_i = 0; lockee_i < graph->__lockee_list_sizes[block_i]; ++lockee_i)
    {
        uint32_t lockee = graph->__lockee_list[block_i][lockee_i];
        ++graph->__lock_counters[lockee];
    }

    pthread_mutex_unlock(&graph->__counters_lock);
}


void lp_lock_graph_unlock(lp_lock_graph_t *graph, uint32_t block_i)
{
    affirm_nullptr(graph,"lock graph");
    affirmf(block_i < graph->blocks_num,"Block index %d is out of range (max: %d)",block_i,graph->blocks_num-1);
    affirmf(graph->commited,"Specified graph must be commited");
    
    pthread_mutex_lock(&graph->__counters_lock);

    for(uint32_t lockee_i = 0; lockee_i < graph->__lockee_list_sizes[block_i]; ++lockee_i)
    {
        uint32_t lockee = graph->__lockee_list[block_i][lockee_i];
        --graph->__lock_counters[lockee];
        pthread_cond_broadcast(&graph->__block_conds[lockee]);
    }

    pthread_mutex_unlock(&graph->__counters_lock);
}