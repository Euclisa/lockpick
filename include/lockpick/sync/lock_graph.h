#ifndef _LOCKPICK_SYNC_LOCK_GRAPH_H
#define _LOCKPICK_SYNC_LOCK_GRAPH_H

#include <lockpick/sync/spinlock_bitset.h>
#include <stdint.h>
#include <pthread.h>


typedef struct lp_lock_graph
{
    uint32_t **__lockee_list;
    uint32_t *__lockee_list_sizes;
    pthread_cond_t *__block_conds;
    pthread_mutex_t __counters_lock;
    uint32_t *__lock_counters;
    uint32_t blocks_num;
    bool commited;
} lp_lock_graph_t;


lp_lock_graph_t *lp_lock_graph_create(uint32_t blocks_num);
void lp_lock_graph_release(lp_lock_graph_t *graph);

void lp_lock_graph_add_dep(lp_lock_graph_t *graph, uint32_t locker, uint32_t lockee);
void lp_lock_graph_add_dep_mutual(lp_lock_graph_t *graph, uint32_t a, uint32_t b);
void lp_lock_graph_commit(lp_lock_graph_t *graph);

void lp_lock_graph_lock(lp_lock_graph_t *graph, uint32_t block_i);
void lp_lock_graph_unlock(lp_lock_graph_t *graph, uint32_t block_i);

uint32_t lp_lock_graph_blocks_num(lp_lock_graph_t *graph);

#endif // _LOCKPICK_SYNC_LOCK_GRAPH_H