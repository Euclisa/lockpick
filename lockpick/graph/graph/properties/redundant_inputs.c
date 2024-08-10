#include <lockpick/graph/traverse.h>
#include <lockpick/affirmf.h>


void __lpg_validate_cb(lpg_graph_t *graph, lpg_node_t *node, bool is_input, void *args)
{
    size_t *reached_inputs_count = args;

    if(is_input)
        ++(*reached_inputs_count);
}


size_t lpg_graph_count_redundant_inputs(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    size_t reached_inputs = 0;
    lpg_graph_traverse_once(graph,__lpg_validate_cb,&reached_inputs);

    return graph->inputs_size - reached_inputs;
}