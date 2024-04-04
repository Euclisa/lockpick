#ifndef _LOCKPICK_GRAPH_TSORT_H
#define _LOCKPICK_GRAPH_TSORT_H

#include <lockpick/graph/graph.h>
#include <lockpick/graph/ocl_graph.h>


#define INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE 3

typedef struct __tsort_init_state
{
    lpg_node_t **const_nodes;
    size_t nodes_count,const_nodes_count;
} __tsort_init_state_t;

void __lpg_tsort_init_state_cb(lpg_graph_t *graph, lpg_node_t *node, bool is_input, void *args);

void lpg_graph_tsort(lpg_graph_t *graph, lpg_node_t ***sorted_nodes);

void lpg_graph_tsort_packed(lpg_graph_t *graph, lpg_node_packed_t **sorted_nodes);

#endif // _LOCKPICK_GRAPH_TSORT_H