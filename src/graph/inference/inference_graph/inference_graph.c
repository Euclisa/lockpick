#include <lockpick/graph/inference/inference_graph.h>
#include <lockpick/graph/count.h>
#include <lockpick/affirmf.h>
#include <lockpick/utility.h>
#include <lockpick/htable.h>


lpg_inference_graph_t *lpg_inference_graph_create(lpg_graph_t *graph, bool gen_inverse_index)
{
    affirm_nullptr(graph,"graph");

    size_t redundant_inputs_num = lpg_graph_count_redundant_inputs(graph);
    affirmf(redundant_inputs_num == 0,
        "Given graph contains %zd redundant input nodes and cannot be converted to ocl graph",redundant_inputs_num);

    size_t inference_graph_size = sizeof(lpg_inference_graph_t);
    lpg_inference_graph_t *inference_graph = (lpg_inference_graph_t*)malloc(inference_graph_size);
    affirm_bad_malloc(inference_graph,"ocl graph",inference_graph_size);

    inference_graph->graph = graph;

    __lpg_inference_graph_tsort_packed(inference_graph,gen_inverse_index);

    affirmf(inference_graph->nodes_num <= LPG_INFERENCE_GRAPH_MAX_NODES_NUM,
        "Number of nodes in the given graph exceeds max number of nodes supported (%zd > %d)",
        inference_graph->nodes_num,LPG_INFERENCE_GRAPH_MAX_NODES_NUM);

    return inference_graph;
}


void lpg_inference_graph_release(lpg_inference_graph_t *inference_graph)
{
    affirm_nullptr(inference_graph,"ocl graph");

    lp_htable_release(inference_graph->index_map);
    if(inference_graph->inv_index_map)
        lp_htable_release(inference_graph->inv_index_map);
    
    free(inference_graph->sorted_nodes);
    free(inference_graph);
}


size_t __lpg_inference_graph_index_map_hsh(const __lpg_inference_graph_index_map_entry_t *entry)
{
    return lp_uni_hash((size_t)entry->node);
}

bool __lpg_inference_graph_index_map_eq(const __lpg_inference_graph_index_map_entry_t *a, const __lpg_inference_graph_index_map_entry_t *b)
{
    return a->node == b->node;
}


inline void __lpg_inference_graph_index_map_insert(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t index)
{
    __lpg_inference_graph_index_map_entry_t entry;
    entry.node = node;
    entry.index = index;
    lp_htable_insert(inference_graph->index_map,&entry);
}


inline void lpg_inference_graph_index_map_find(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t *result)
{
    __lpg_inference_graph_index_map_entry_t entry;
    entry.node = node;
    __lpg_inference_graph_index_map_entry_t found;
    lp_htable_find(inference_graph->index_map,&entry,&found);
    *result = found.index;
}


size_t __lpg_inference_graph_inv_index_map_hsh(const __lpg_inference_graph_index_map_entry_t *entry)
{
    return lp_uni_hash(entry->index);
}

bool __lpg_inference_graph_inv_index_map_eq(const __lpg_inference_graph_index_map_entry_t *a, const __lpg_inference_graph_index_map_entry_t *b)
{
    return a->index == b->index;
}


inline void __lpg_inference_graph_inv_index_map_insert(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t index)
{
    __lpg_inference_graph_index_map_entry_t entry;
    entry.node = node;
    entry.index = index;
    lp_htable_insert(inference_graph->inv_index_map,&entry);
}


inline void lpg_inference_graph_inv_index_map_find(lpg_inference_graph_t *inference_graph, lpg_inference_graph_index_t index, lpg_node_t **result)
{
    __lpg_inference_graph_index_map_entry_t entry;
    entry.index = index;
    __lpg_inference_graph_index_map_entry_t found;
    lp_htable_find(inference_graph->inv_index_map,&entry,&found);
    *result = found.node;
}
