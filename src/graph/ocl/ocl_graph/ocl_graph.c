#include <lockpick/graph/ocl_graph.h>
#include <lockpick/graph/count.h>
#include <lockpick/affirmf.h>
#include <lockpick/utility.h>
#include <lockpick/htable.h>


lpg_ocl_graph_t *lpg_ocl_graph_create(lpg_graph_t *graph, bool gen_reverse_index)
{
    affirm_nullptr(graph,"graph");

    size_t redundant_inputs_num = lpg_graph_count_redundant_inputs(graph);
    affirmf(redundant_inputs_num == 0,
        "Given graph contains %zd redundant input nodes and cannot be converted to ocl graph",redundant_inputs_num);

    size_t ocl_graph_size = sizeof(lpg_ocl_graph_t);
    lpg_ocl_graph_t *ocl_graph = (lpg_ocl_graph_t*)malloc(ocl_graph_size);
    affirm_bad_malloc(ocl_graph,"ocl graph",ocl_graph_size);

    ocl_graph->graph = graph;

    __lpg_ocl_graph_tsort_packed(ocl_graph,gen_reverse_index);

    affirmf(ocl_graph->nodes_num <= LPG_OCL_GRAPH_MAX_NODES_NUM,
        "Number of nodes in the given graph exceeds max number of nodes supported (%zd > %d)",
        ocl_graph->nodes_num,LPG_OCL_GRAPH_MAX_NODES_NUM);

    return ocl_graph;
}


void lpg_ocl_graph_release(lpg_ocl_graph_t *ocl_graph)
{
    affirm_nullptr(ocl_graph,"ocl graph");

    lp_htable_release(ocl_graph->index_map);
    if(ocl_graph->rev_index_map)
        lp_htable_release(ocl_graph->rev_index_map);
    
    free(ocl_graph->sorted_nodes);
    free(ocl_graph);
}


cl_char __lpg_node_packed_type_from_node(const lpg_node_t *node)
{
    cl_char packed_type = 0;
    switch(node->type)
    {
        case LPG_NODE_TYPE_AND:
        case LPG_NODE_TYPE_OR:
        case LPG_NODE_TYPE_NOT:
        case LPG_NODE_TYPE_XOR:
            packed_type = (cl_char)node->type;
            break;
        
        case LPG_NODE_TYPE_CONST:
            if(lpg_node_value(node))
                packed_type = (cl_char)LPG_NODE_PACKED_TYPE_TRUE;
            else
                packed_type = (cl_char)LPG_NODE_PACKED_TYPE_FALSE;
            break;
    }

    return packed_type;
}


inline lpg_node_packed_t __lpg_node_packed_from_node(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node)
{
    lpg_node_packed_t packed_node;

    size_t parents_num = lpg_node_get_parents_num(node);
    lpg_node_t **parents = lpg_node_parents(node);
    for(size_t parent_i = 0; parent_i < parents_num; ++parent_i)
    {
        uint16_t packed_parent_i;
        lpg_ocl_graph_index_map_find(ocl_graph,parents[parent_i],&packed_parent_i);
        packed_node.parents[parent_i] = packed_parent_i;
    }
    packed_node.type = __lpg_node_packed_type_from_node(node);

    return packed_node;
}


inline lpg_node_packed_t __lpg_node_packed_from_const_node(const lpg_node_t *node)
{
    lpg_node_packed_t packed_node;
    packed_node.type = __lpg_node_packed_type_from_node(node);

    affirmf_debug(packed_node.type == LPG_NODE_PACKED_TYPE_TRUE || packed_node.type == LPG_NODE_PACKED_TYPE_FALSE,
            "Expected node of terminal type but type %d was given",(uint32_t)packed_node.type);

    return packed_node;
}


inline lpg_node_packed_t __lpg_node_packed_from_input_node(const lpg_node_t *node)
{
    lpg_node_packed_t packed_node;
    packed_node.type = LPG_NODE_PACKED_TYPE_INPUT;

    return packed_node;
}


size_t __lpg_ocl_graph_index_map_hsh(const __lpg_ocl_graph_index_map_entry_t *entry)
{
    return lp_uni_hash((size_t)entry->node);
}

bool __lpg_ocl_graph_index_map_eq(const __lpg_ocl_graph_index_map_entry_t *a, const __lpg_ocl_graph_index_map_entry_t *b)
{
    return a->node == b->node;
}


inline void __lpg_ocl_graph_index_map_insert(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node, uint16_t index)
{
    __lpg_ocl_graph_index_map_entry_t entry;
    entry.node = node;
    entry.index = index;
    lp_htable_insert(ocl_graph->index_map,&entry);
}


inline void lpg_ocl_graph_index_map_find(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node, uint16_t *result)
{
    __lpg_ocl_graph_index_map_entry_t entry;
    entry.node = node;
    __lpg_ocl_graph_index_map_entry_t found;
    lp_htable_find(ocl_graph->index_map,&entry,&found);
    *result = found.index;
}


size_t __lpg_ocl_graph_rev_index_map_hsh(const __lpg_ocl_graph_index_map_entry_t *entry)
{
    return lp_uni_hash(entry->index);
}

bool __lpg_ocl_graph_rev_index_map_eq(const __lpg_ocl_graph_index_map_entry_t *a, const __lpg_ocl_graph_index_map_entry_t *b)
{
    return a->index == b->index;
}


inline void __lpg_ocl_graph_rev_index_map_insert(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node, uint16_t index)
{
    __lpg_ocl_graph_index_map_entry_t entry;
    entry.node = node;
    entry.index = index;
    lp_htable_insert(ocl_graph->rev_index_map,&entry);
}


inline void lpg_ocl_graph_rev_index_map_find(lpg_ocl_graph_t *ocl_graph, uint16_t index, lpg_node_t **result)
{
    __lpg_ocl_graph_index_map_entry_t entry;
    entry.index = index;
    __lpg_ocl_graph_index_map_entry_t found;
    lp_htable_find(ocl_graph->rev_index_map,&entry,&found);
    *result = found.node;
}
