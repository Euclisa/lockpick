#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>


lp_slab_t *__lpg_node_create_slab(size_t total_entries)
{
    lp_slab_t *slab = lp_slab_create(total_entries,sizeof(lpg_node_t));

    return slab;
}


lpg_graph_t *lpg_graph_create(const char *name, size_t inputs_size, size_t outputs_size, size_t max_nodes)
{
    affirmf(name,"Expected valid pointer on graph name str");

    lpg_graph_t *graph = (lpg_graph_t*)malloc(sizeof(lpg_graph_t));
    affirmf(graph,"Failed to allocate space for graph '%s'",name);

    graph->name = name;

    graph->slab = __lpg_node_create_slab(max_nodes);
    affirmf(graph->slab,"Failed to create slab for %ld nodes",max_nodes);

    graph->inputs = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*inputs_size);
    affirmf(graph->inputs,"Failed to allocate space for input %ld input nodes",inputs_size);
    graph->inputs_size = inputs_size;

    graph->outputs = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*outputs_size);
    affirmf(graph->outputs,"Failed to allocate space for input %ld input nodes",outputs_size);
    graph->outputs_size = outputs_size;

    for(size_t in_i = 0; in_i < inputs_size; ++in_i)
        graph->inputs[in_i] = lpg_node_var(graph);

    return graph;
}
