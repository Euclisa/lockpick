#ifndef _LOCKPICK_SYNC_LOCK_GRAPH_H
#define _LOCKPICK_SYNC_LOCK_GRAPH_H

#include <lockpick/sync/spinlock_bitset.h>
#include <stdint.h>
#include <pthread.h>


typedef struct lp_lock_graph
{
    uint32_t **__build_adj_list;
    uint32_t *__adj_list;
    pthread_cond_t *__block_conds;
    pthread_mutex_t *__block_locks;
    uint32_t *__adj_list_offsets;
    uint32_t *__lock_counters;
    lp_spinlock_bitset_t *__lock_counters_spins;
    uint32_t __blocks_num;
} lp_lock_graph_t;


lp_lock_graph_t *lp_lock_graph_init(uint32_t blocks_num);

#endif // _LOCKPICK_SYNC_LOCK_GRAPH_H