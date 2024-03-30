#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


inline size_t lpg_graph_nodes_count_super(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    return slab->__total_entries - slab->__total_free;
}


inline size_t lpg_graph_operators_count_super(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    return lpg_graph_nodes_count_super(graph) - graph->inputs_size;
}


static void __lpg_graph_nodes_count_cb(lpg_graph_t *graph, lpg_node_t *node, void *args)
{
    _Atomic size_t *count = args;
    ++(*count);
}


size_t lpg_graph_nodes_count(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    _Atomic size_t count = 0;
    lpg_graph_traverse_once(graph,__lpg_graph_nodes_count_cb,&count);

    return count;
}
