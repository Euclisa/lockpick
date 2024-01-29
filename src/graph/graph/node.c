#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


const uint16_t lpg_node_type_operands_num[] = {2, 2, 1, 2, 0, 0, 0};

inline lpg_node_t **lpg_node_parents(const lpg_node_t *node)
{
    return (lpg_node_t**)(node->__parents_computed_value & __LPG_NODE_PARENTS_MASK);
}

inline void __lpg_node_set_parents(lpg_node_t *node, lpg_node_t **parents)
{
    node->__parents_computed_value = (uintptr_t)parents | 
            (node->__parents_computed_value & (~__LPG_NODE_PARENTS_MASK));
}


inline bool lpg_node_computed(const lpg_node_t *node)
{
    return node->__parents_computed_value & __LPG_NODE_COMPUTED_MASK;
}

inline void __lpg_node_set_computed(lpg_node_t *node, bool computed)
{
    if(computed)
        node->__parents_computed_value |= __LPG_NODE_COMPUTED_MASK;
    else
        node->__parents_computed_value &= ~__LPG_NODE_COMPUTED_MASK;
}


inline bool lpg_node_value(const lpg_node_t *node)
{
    return node->__parents_computed_value & __LPG_NODE_VALUE_MASK;
}

inline void __lpg_node_set_value(lpg_node_t *node, bool value)
{
    if(value)
        node->__parents_computed_value |= __LPG_NODE_VALUE_MASK;
    else
        node->__parents_computed_value &= ~__LPG_NODE_VALUE_MASK;
}


inline void __lpg_add_child(lpg_node_t *node, lpg_node_t *child)
{
    ++node->children_size;
    node->children = realloc(node->children,sizeof(lpg_node_t*)*node->children_size);
    node->children[node->children_size-1] = child;
}


inline bool __lpg_node_belongs_to_graph(const lpg_graph_t *graph, const lpg_node_t *node)
{
    lpg_node_t *graph_slab_base = graph->slab->__buffer;
    lpg_node_t *graph_slab_end = graph_slab_base+graph->slab->__total_entries*sizeof(lpg_node_t);

    return node >= graph_slab_base && node < graph_slab_end;
}


inline bool lpg_node_validate_fetch(const lpg_graph_t *graph, const lpg_node_t *node)
{
    affirmf(__lpg_node_belongs_to_graph(graph,node),"Specified node does not belong to the given graph");
    affirmf(lpg_node_computed(node),"Attempt to read value of node that had not been computed before");
    
    return lpg_node_value(node);
}


inline void __lpg_node_release(lpg_node_t *node)
{
    lpg_node_t **parents = lpg_node_parents(node);

    if(parents)
        free(parents);
    
    if(node->children)
        free(node->children);
}