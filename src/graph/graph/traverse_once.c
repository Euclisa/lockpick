#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/list.h>
#include <lockpick/container_of.h>
#include <lockpick/htable.h>
#include <lockpick/utility.h>
#include <lockpick/affinity.h>
#include <lockpick/sync/spinlock_bitset.h>
#include <lockpick/sync/visit_table.h>
#include <pthread.h>
#include <stdbool.h>


typedef struct __lp_node_stack
{
    lp_list_t __list;
    lpg_node_t *node;
} __lp_node_stack_t;


static inline void __lpg_node_stack_lock_push(__lp_node_stack_t **stack, lpg_node_t *node, lp_spinlock_bitset_t *spins, size_t lock_i)
{
    __lp_node_stack_t *entry = (__lp_node_stack_t*)malloc(sizeof(__lp_node_stack_t));
    entry->node = node;

    lp_spinlock_bitset_lock(spins,lock_i);
    lp_list_t *old_head = *stack ? &(*stack)->__list : NULL;
    lp_list_push_head(&old_head,&entry->__list);
    *stack = container_of(old_head,__lp_node_stack_t,__list);
    lp_spinlock_bitset_unlock(spins,lock_i);
}

static inline __lp_node_stack_t *__lpg_node_stack_lock_pop_and_undeplete(__lp_node_stack_t **stack, lp_spinlock_bitset_t *spins, size_t lock_i, _Atomic uint32_t *depleted_threads_cnt)
{
    lp_spinlock_bitset_lock(spins,lock_i);
    __lp_node_stack_t *top = (*stack);
    if(top)
    {
        lp_list_t *old_head = &(*stack)->__list;
        lp_list_remove_head(&old_head);
        *stack = container_of(old_head,__lp_node_stack_t,__list);
        --(*depleted_threads_cnt);
    }
    lp_spinlock_bitset_unlock(spins,lock_i);

    return top;
}


static inline __lp_node_stack_t *__lpg_node_stack_lock_pop_and_deplete(__lp_node_stack_t **stack, lp_spinlock_bitset_t *spins, size_t lock_i, _Atomic uint32_t *depleted_threads_cnt)
{
    lp_spinlock_bitset_lock(spins,lock_i);
    __lp_node_stack_t *top = (*stack);
    if(top)
    {
        lp_list_t *old_head = &(*stack)->__list;
        lp_list_remove_head(&old_head);
        *stack = container_of(old_head,__lp_node_stack_t,__list);
    }
    else
        ++(*depleted_threads_cnt);
    lp_spinlock_bitset_unlock(spins,lock_i);

    return top;
}


size_t __lpg_graph_nodes_hsh(const lpg_node_t **node)
{
    return lp_uni_hash((size_t)(*node));
}

bool __lpg_graph_nodes_eq(const lpg_node_t **a, const lpg_node_t **b)
{
    return *a == *b;
}


typedef struct __lpg_graph_traverse_thr_args_common
{
    lpg_graph_t *graph;
    lpg_traverse_cb_t cb;
    void *cb_args;
    __lp_node_stack_t **trav_stacks;
    lp_spinlock_bitset_t *spins;
    lp_visit_table_t *visited;
    lp_htable_t *inputs;
    _Atomic uint32_t *depleted_threads_cnt;
    uint32_t total_threads;
    pthread_cond_t *term_cond;

} __lpg_graph_traverse_thr_args_common_t;

typedef struct __lpg_graph_traverse_thr_args
{
    __lpg_graph_traverse_thr_args_common_t *common_args;
    uint32_t current_thread_i;
} __lpg_graph_traverse_thr_args_t;


static inline bool __is_finished(_Atomic uint32_t *depleted_threads_cnt, uint32_t total_threads, pthread_cond_t *term_cond)
{
    if(*depleted_threads_cnt == total_threads)
    {
        pthread_cond_signal(term_cond);
        return true;
    }

    return false;
}


static inline __lp_node_stack_t *__borrow_node(__lp_node_stack_t **trav_stacks, uint32_t current_thread_i, lp_visit_table_t *visited, lp_spinlock_bitset_t *spins, _Atomic uint32_t *depleted_threads_cnt, uint32_t total_threads, pthread_cond_t *term_cond)
{
    for(uint32_t thr_i = 0;; thr_i = (thr_i+1) % total_threads)
    {
        if(current_thread_i == thr_i)
            continue;

        __lp_node_stack_t *popped = __lpg_node_stack_lock_pop_and_undeplete(&trav_stacks[thr_i],spins,thr_i,depleted_threads_cnt);
        if(!popped)
            continue;
        lpg_node_t *node = popped->node;
        if(!lp_visit_table_find(visited,&node,NULL))
            return popped;
        else
        {
            free(popped);
            ++(*depleted_threads_cnt);
            if(__is_finished(depleted_threads_cnt,total_threads,term_cond))
                return NULL;
        }
    }
}


void *__lpg_graph_traverse_node_once_thr(__lpg_graph_traverse_thr_args_t *args)
{
    uint32_t current_thread_i = args->current_thread_i;
    lpg_traverse_cb_t cb = args->common_args->cb;
    void *cb_args = args->common_args->cb_args;
    _Atomic uint32_t *depleted_threads_cnt = args->common_args->depleted_threads_cnt;
    lpg_graph_t *graph = args->common_args->graph;
    lp_htable_t *inputs = args->common_args->inputs;
    lp_spinlock_bitset_t *spins = args->common_args->spins;
    uint32_t total_threads = args->common_args->total_threads;
    __lp_node_stack_t **trav_stacks = args->common_args->trav_stacks;
    lp_visit_table_t *visited = args->common_args->visited;
    pthread_cond_t *term_cond = args->common_args->term_cond;

    affirmf(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL),
        "Failed to set cancel type of thread %d to PTHREAD_CANCEL_ASYNCHRONOUS",current_thread_i);

    while(true)
    {
        __lp_node_stack_t *popped = __lpg_node_stack_lock_pop_and_deplete(&trav_stacks[current_thread_i],spins,current_thread_i,depleted_threads_cnt);
        if(!popped)
        {
            if(__is_finished(depleted_threads_cnt,total_threads,term_cond))
                return NULL;
            popped = __borrow_node(trav_stacks,current_thread_i,visited,spins,depleted_threads_cnt,total_threads,term_cond);
            if(!popped)
                return NULL;
        }
    
        lpg_node_t *curr_node = popped->node;
        free(popped);

        if(!lp_visit_table_insert(visited,&curr_node))
            continue;
        
        cb(graph,curr_node,cb_args);

        if(lp_htable_find(inputs,&curr_node,NULL))
            continue;

        uint16_t curr_node_parents_num = lpg_node_get_parents_num(curr_node);
        lpg_node_t **curr_node_parents = lpg_node_parents(curr_node);
        for(uint16_t parent_i = 0; parent_i < curr_node_parents_num; ++parent_i)
        {
            lpg_node_t *parent = curr_node_parents[parent_i];
            bool is_parent_visited = lp_visit_table_find(visited,&parent,NULL);
            if(!is_parent_visited)
                __lpg_node_stack_push_sync(&trav_stacks[current_thread_i],parent,spins,current_thread_i);
        }

    }
}


void lpg_graph_traverse_once(lpg_graph_t *graph, lpg_traverse_cb_t cb, void *cb_args)
{
    affirm_nullptr(graph,"graph");

    lp_visit_table_t *visited = lp_visit_table_create_max_el(
        lpg_graph_nodes_count(graph),
        sizeof(lpg_node_t*),
        (size_t (*)(const void *))__lpg_graph_nodes_hsh,
        (bool (*)(const void *,const void *))__lpg_graph_nodes_eq);

    lp_htable_t *inputs = lp_htable_create_el_num(
            graph->inputs_size,
            sizeof(lpg_node_t*),
            (size_t (*)(const void *))__lpg_graph_nodes_hsh,
            (bool (*)(const void *,const void *))__lpg_graph_nodes_eq);

    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
        lp_htable_insert(inputs,&graph->inputs[in_node_i]);

    uint32_t threads_num = lp_affinity_cpu_count;
    pthread_t *threads = (pthread_t*)malloc(threads_num*sizeof(pthread_t));

    lp_spinlock_bitset_t *spins = lp_spinlock_bitset_create(threads_num);

    __lp_node_stack_t **trav_stacks = (__lp_node_stack_t**)calloc(threads_num,sizeof(__lp_node_stack_t*));
    for(size_t out_i = 0; out_i < graph->outputs_size; ++out_i)
        __lpg_node_stack_push(&trav_stacks[out_i%threads_num],graph->outputs[out_i]);
    
    _Atomic uint32_t depleted_threads_cnt = 0;

    pthread_cond_t term_cond;
    affirmf(pthread_cond_init(&term_cond,NULL),"Failed to create threads termination condition");

    __lpg_graph_traverse_thr_args_common_t common_args;
    common_args.cb = cb;
    common_args.cb_args = cb_args;
    common_args.depleted_threads_cnt = depleted_threads_cnt;
    common_args.graph = graph;
    common_args.inputs = inputs;
    common_args.spins = spins;
    common_args.total_threads = threads_num;
    common_args.trav_stacks = trav_stacks;
    common_args.visited = visited;
    common_args.term_cond = &term_cond;

    __lpg_graph_traverse_thr_args_t *args = (__lpg_graph_traverse_thr_args_t*)malloc(threads_num*sizeof(__lpg_graph_traverse_thr_args_t));

    for(uint32_t thr_i = 0; thr_i < threads_num; ++thr_i)
    {
        args[thr_i].common_args = &common_args;
        args[thr_i].current_thread_i = thr_i;
        affirmf(!pthread_create(&threads[thr_i],NULL,__lpg_graph_traverse_node_once_thr,&args[thr_i]),
            "Failed to create thread %d",thr_i);
        affirmf(!pthread_setaffinity_np(threads[thr_i],sizeof(cpu_set_t),&lp_affinity_cpus[thr_i]),
            "Failed to set cpu affinity for thread %d",thr_i);
    }

    pthread_mutex_t term_lock;
    pthread_mutex_init(&term_lock,NULL);

    pthread_mutex_lock(&term_lock);
    pthread_cond_wait(&term_cond,&term_lock);
    pthread_mutex_unlock(&term_lock);

    for(uint32_t thr_i = 0; thr_i < threads_num; ++thr_i)
        affirmf(!pthread_cancel(threads[thr_i]),"Failed to cancel thread %d",thr_i);
    
    pthread_cond_destroy(&term_cond);
    pthread_mutex_destroy(&term_lock);
    free(threads);
    free(args);
    lp_spinlock_bitset_release(spins);
    lp_visit_table_release(visited);
    lp_htable_release(inputs);
}