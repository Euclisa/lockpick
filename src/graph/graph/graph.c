#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>
#include <string.h>


lp_slab_t *__lpg_graph_slab(const lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    return (lp_slab_t*)(graph->__slab_super & __LPG_GRAPH_SLAB_MASK);
}

void __lpg_graph_set_slab(lpg_graph_t *graph, lp_slab_t *slab)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(slab,"graph slab");

    graph->__slab_super = (uintptr_t)slab | (graph->__slab_super & (~__LPG_GRAPH_SLAB_MASK));
}


bool lpg_graph_is_super(const lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    return (bool)(graph->__slab_super & __LPG_GRAPH_SUPER_MASK);
}

void __lpg_graph_set_super(lpg_graph_t *graph, bool super)
{
    affirm_nullptr(graph,"graph");
    
    if(super)
        graph->__slab_super |= __LPG_GRAPH_SUPER_MASK;
    else
        graph->__slab_super &= ~__LPG_GRAPH_SUPER_MASK;
}


lp_slab_t *__lpg_node_create_slab(size_t total_entries)
{
    lp_slab_t *slab = lp_slab_create(total_entries,sizeof(lpg_node_t));

    return slab;
}

lpg_graph_t *lpg_graph_create(const char *name, size_t inputs_size, size_t outputs_size, size_t max_nodes)
{
    affirm_nullptr(name,"graph name string");

    lpg_graph_t *graph = (lpg_graph_t*)malloc(sizeof(lpg_graph_t));
    affirmf(graph,"Failed to allocate space for graph '%s'",name);

    size_t name_str_len = strlen(name);
    graph->name = (char*)malloc(name_str_len+1);
    strcpy(graph->name,name);

    lp_slab_t *slab = __lpg_node_create_slab(max_nodes);
    affirmf(slab,"Failed to create slab for %ld nodes",max_nodes);
    __lpg_graph_set_slab(graph,slab);
    __lpg_graph_set_super(graph,true);

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


static inline void __lpg_graph_release_slab_callback(void *entry_ptr, void *args)
{
    lpg_node_t *node = (lpg_node_t*)entry_ptr;
    
    __lpg_node_release(node);
}

void lpg_graph_release(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lp_slab_exec(slab,__lpg_graph_release_slab_callback,NULL);
    lp_slab_release(slab);
    free(graph->name);
    free(graph->inputs);
    free(graph->outputs);
    free(graph);
}


void lpg_graph_release_node(lpg_graph_t *graph, lpg_node_t *node)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(node,"node");
    affirmf(__lpg_graph_is_native_node(graph,node),"Specified node does not belong to the given graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lp_slab_free(slab,node);
    __lpg_node_release(node);
}


static inline void __lpg_graph_reset_slab_callback(void *entry_ptr, void *args)
{
    lpg_node_t *node = (lpg_node_t*)entry_ptr;

    if(node->type != LPG_NODE_TYPE_VAR)
        __lpg_node_set_computed(node,false);
}

inline void lpg_graph_reset(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lp_slab_exec(slab,__lpg_graph_reset_slab_callback,NULL);    
}


inline size_t lpg_graph_nodes_count(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    return slab->__total_entries - slab->__total_free;
}


inline size_t lpg_graph_operators_count(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    return lpg_graph_nodes_count(graph) - graph->inputs_size;
}


inline bool __lpg_graph_is_native_node(const lpg_graph_t *graph, const lpg_node_t *node)
{
    const lp_slab_t *slab = __lpg_graph_slab(graph);
    const lpg_node_t *graph_slab_base = slab->__buffer;
    const lpg_node_t *graph_slab_end = graph_slab_base+slab->__total_entries*sizeof(lpg_node_t);

    return node >= graph_slab_base && node < graph_slab_end;
}