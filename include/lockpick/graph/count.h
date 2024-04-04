#ifndef _LOCKPICK_GRAPH_COUNT_H
#define _LOCKPICK_GRAPH_COUNT_H

#include <lockpick/graph/graph.h>


size_t lpg_graph_nodes_count_super(lpg_graph_t *graph);

size_t lpg_graph_nodes_count_mt(lpg_graph_t *graph);
size_t lpg_graph_nodes_count(lpg_graph_t *graph);

size_t lpg_graph_count_dangling_nodes(lpg_graph_t *graph);

size_t lpg_graph_count_redundant_inputs(lpg_graph_t *graph);

#endif // _LOCKPICK_GRAPH_COUNT_H