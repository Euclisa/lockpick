#include <lockpick/sync/lock_graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <stdlib.h>


lp_lock_graph_t *lp_lock_graph_init(uint32_t blocks_num)
{
    size_t graph_size = sizeof(lp_lock_graph_t);
    lp_lock_graph_t *graph = (lp_lock_graph_t*)malloc(graph_size);
    return_set_errno_on(!graph,NULL,ENOMEM);

    size_t build_adj_list_size = blocks_num*sizeof(uint32_t*);
    graph->__build_adj_list = (uint32_t**)malloc(build_adj_list_size);
    return_set_errno_on(!graph->__build_adj_list,NULL,ENOMEM);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
    {
        graph->__build_adj_list[block_i] = (uint32_t*)malloc(0);
        return_set_errno_on(!graph->__build_adj_list[block_i],NULL,ENOMEM);
    }

    graph->__lock_counters = (uint32_t*)calloc(blocks_num,sizeof(uint32_t));
    return_set_errno_on(!graph->__lock_counters,NULL,ENOMEM);

    graph->__lock_counters_spins = lp_spinlock_bitset_init(blocks_num);
    return_on(!graph->__lock_counters_spins,NULL);

    size_t block_conds_size = blocks_num*sizeof(pthread_cond_t);
    graph->__block_conds = (pthread_cond_t*)malloc(block_conds_size);
    return_set_errno_on(!graph->__block_conds,NULL,ENOMEM);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
        return_on(pthread_cond_init(&graph->__block_conds[block_i],NULL) != 0,NULL);

    size_t block_locks_size = blocks_num*sizeof(pthread_mutex_t);
    graph->__block_locks = (pthread_mutex_t*)malloc(block_locks_size);
    return_set_errno_on(!graph->__block_locks,NULL,ENOMEM);
    for(uint32_t block_i = 0; block_i < blocks_num; ++block_i)
        return_on(pthread_mutex_init(&graph->__block_locks[block_i],NULL) != 0,NULL);

    graph->__adj_list = NULL;
    graph->__blocks_num = blocks_num;

    return graph;
}