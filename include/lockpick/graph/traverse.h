#ifndef _LOCKPICK_GRAPH_TRAVERSE_H
#define _LOCKPICK_GRAPH_TRAVERSE_H

#include <lockpick/graph/graph.h>


typedef void (*lpg_traverse_cb_t)(lpg_graph_t *graph, lpg_node_t *node, bool is_input, void *args);
void lpg_graph_traverse_node(lpg_graph_t *graph, lpg_node_t *node, lpg_traverse_cb_t enter_cb, void *enter_cb_args, lpg_traverse_cb_t leave_cb, void *leave_cb_args);
void lpg_graph_traverse(lpg_graph_t *graph, lpg_traverse_cb_t enter_cb, void *enter_cb_args, lpg_traverse_cb_t leave_cb, void *leave_cb_args);
void lpg_graph_traverse_once(lpg_graph_t *graph, lpg_traverse_cb_t cb, void *cb_args);
void lpg_graph_traverse_once_sync(lpg_graph_t *graph, lpg_traverse_cb_t cb, void *cb_args);

#endif // _LOCKPICK_GRAPH_TRAVERSE_H