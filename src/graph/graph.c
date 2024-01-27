#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>
#include <string.h>


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

    size_t name_str_len = strlen(name);
    graph->name = (char*)malloc(name_str_len+1);
    strcpy(graph->name,name);

    graph->slab = __lpg_node_create_slab(max_nodes);
    affirmf(graph->slab,"Failed to create slab for %ld nodes",max_nodes);

    graph->inputs = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*inputs_size);
    affirmf(graph->inputs,"Failed to allocate space for input %ld input nodes",inputs_size);
    graph->inputs_size = inputs_size;

    graph->outputs = (lpg_node_t**)calloc(outputs_size,sizeof(lpg_node_t*));
    affirmf(graph->outputs,"Failed to allocate space for input %ld input nodes",outputs_size);
    graph->outputs_size = outputs_size;

    for(size_t in_i = 0; in_i < inputs_size; ++in_i)
        graph->inputs[in_i] = lpg_node_var(graph);

    return graph;
}


static inline void __lpg_graph_release_slab_callback(void *ptr)
{
    lpg_node_t *node = (lpg_node_t*)ptr;
    lpg_node_t **parents = lpg_node_parents(node);

    if(parents)
        free(parents);
    
    if(node->children)
        free(node->children);
}

void lpg_graph_release(lpg_graph_t *graph)
{
    affirmf(graph,"Expected valid pointer on graph structure but null was given");
    lp_slab_exec(graph->slab,__lpg_graph_release_slab_callback);
    lp_slab_release(graph->slab);
    free(graph->name);
    free(graph->inputs);
    free(graph->outputs);
    free(graph);
}

static inline void __lpg_graph_reset_slab_callback(void *ptr)
{
    lpg_node_t *node = (lpg_node_t*)ptr;

    if(node->type != LPG_NODE_TYPE_VAR)
        __lpg_node_set_computed(node,false);
}

inline void lpg_graph_reset(lpg_graph_t *graph)
{
    affirmf(graph,"Expected valid graph pointer but null was given");

    lp_slab_exec(graph->slab,__lpg_graph_reset_slab_callback);    
}


inline void lpg_graph_compute(lpg_graph_t *graph)
{
    affirmf(graph,"Expected valid graph pointer but null was given");

    for(size_t node_i = 0; node_i < graph->outputs_size; ++node_i)
    {
        affirmf(graph->outputs[node_i],"Attempt to compute null graph output a index %ld."
                                    "Is graph assembled properly?",node_i);
        lpg_node_compute(graph->outputs[node_i]);
    }
}