#include <lockpick/sync/lock_graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <lockpick/emalloc.h>
#include <stdlib.h>


lp_lock_graph_t *lp_lock_graph_create(uint32_t blocks_num)
{
    lp_lock_graph_t *graph = (lp_lock_graph_t*)emalloc(1,sizeof(lp_lock_graph_t),NULL);

    graph->__lockee_list = (uint32_t**)emalloc(blocks_num,sizeof(uint32_t*),NULL);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
        graph->__lockee_list[block_i] = (uint32_t*)emalloc(0,sizeof(uint32_t),NULL);

    graph->__lockee_list_sizes = (uint32_t*)ecalloc(blocks_num,sizeof(uint32_t),NULL);

    graph->__lock_counters = (uint32_t*)ecalloc(blocks_num,sizeof(uint32_t),NULL);

    graph->__block_conds = (pthread_cond_t*)emalloc(blocks_num,sizeof(pthread_cond_t),NULL);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
        return_on(pthread_cond_init(&graph->__block_conds[block_i],NULL) != 0,NULL);

    return_on(pthread_mutex_init(&graph->__counters_lock,NULL) != 0,NULL);

    graph->__blocks_num = blocks_num;
    graph->__commited = false;

    return graph;
}


bool lp_lock_graph_release(lp_lock_graph_t *graph)
{
    if(!graph)
        return_set_errno(false,EINVAL);

    for(uint32_t block_i = 0; block_i < graph->__blocks_num; ++block_i)
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

    return true;
}


static inline bool __lp_lock_graph_add_dep(lp_lock_graph_t *graph, uint32_t locker, uint32_t lockee)
{
    uint32_t curr_locker_list_size = graph->__lockee_list_sizes[locker];
    for(uint32_t lockee_i = 0; lockee_i < curr_locker_list_size; ++lockee_i)
        return_set_errno_on(graph->__lockee_list[locker][lockee_i] == lockee,false,EINVAL);

    graph->__lockee_list[locker] = (uint32_t*)erealloc(graph->__lockee_list[locker],curr_locker_list_size+1,sizeof(uint32_t),false);
    graph->__lockee_list[locker][curr_locker_list_size] = lockee;
    ++graph->__lockee_list_sizes[locker];

    return true;
}


bool lp_lock_graph_add_dep(lp_lock_graph_t *graph, uint32_t locker, uint32_t lockee)
{
    if(!graph || locker >= graph->__blocks_num || lockee >= graph->__blocks_num)
        return_set_errno(false,EINVAL);
    if(graph->__commited)
        return_set_errno(false,EBUSY);
    
    return_on(!__lp_lock_graph_add_dep(graph,locker,lockee),false);

    return true;
}


bool lp_lock_graph_add_dep_mutual(lp_lock_graph_t *graph, uint32_t a, uint32_t b)
{
    if(!graph || a >= graph->__blocks_num || b >= graph->__blocks_num)
        return_set_errno(false,EINVAL);
    if(graph->__commited)
        return_set_errno(false,EBUSY);
    
    return_on(!__lp_lock_graph_add_dep(graph,a,b),false);
    if(a != b)
        return_on(!__lp_lock_graph_add_dep(graph,b,a),false);

    return true;
}


inline bool lp_lock_graph_commit(lp_lock_graph_t *graph)
{
    if(!graph)
        return_set_errno(false,EINVAL);
    if(graph->__commited)
        return_set_errno(false,EBUSY);

    graph->__commited = true; 

    return true;
}


bool lp_lock_graph_lock(lp_lock_graph_t *graph, uint32_t block_i)
{
    if(!graph || block_i >= graph->__blocks_num)
        return_set_errno(false,EINVAL);
    if(!graph->__commited)
        return_set_errno(false,EBUSY);

    pthread_mutex_lock(&graph->__counters_lock);

    while(graph->__lock_counters[block_i] != 0)
        pthread_cond_wait(&graph->__block_conds[block_i],&graph->__counters_lock);

    for(uint32_t lockee_i = 0; lockee_i < graph->__lockee_list_sizes[block_i]; ++lockee_i)
    {
        uint32_t lockee = graph->__lockee_list[block_i][lockee_i];
        ++graph->__lock_counters[lockee];
    }

    pthread_mutex_unlock(&graph->__counters_lock);

    return true;
}


bool lp_lock_graph_unlock(lp_lock_graph_t *graph, uint32_t block_i)
{
    if(!graph || block_i >= graph->__blocks_num)
        return_set_errno(false,EINVAL);
    if(!graph->__commited)
        return_set_errno(false,EBUSY);
    
    pthread_mutex_lock(&graph->__counters_lock);

    for(uint32_t lockee_i = 0; lockee_i < graph->__lockee_list_sizes[block_i]; ++lockee_i)
    {
        uint32_t lockee = graph->__lockee_list[block_i][lockee_i];
        --graph->__lock_counters[lockee];
        pthread_cond_broadcast(&graph->__block_conds[lockee]);
    }

    pthread_mutex_unlock(&graph->__counters_lock);

    return true;
}


inline uint32_t lp_lock_graph_blocks_num(lp_lock_graph_t *graph)
{
    return graph->__blocks_num;
}