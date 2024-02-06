#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


inline lpg_node_t **lpg_node_parents(const lpg_node_t *node)
{
    return (lpg_node_t**)(node->__parents_value & __LPG_NODE_PARENTS_MASK);
}

inline void __lpg_node_set_parents(lpg_node_t *node, lpg_node_t **parents)
{
    node->__parents_value = (uintptr_t)parents | 
            (node->__parents_value & (~__LPG_NODE_PARENTS_MASK));
}


inline bool lpg_node_value(const lpg_node_t *node)
{
    return node->__parents_value & __LPG_NODE_VALUE_MASK;
}

inline void __lpg_node_set_value(lpg_node_t *node, bool value)
{
    if(value)
        node->__parents_value |= __LPG_NODE_VALUE_MASK;
    else
        node->__parents_value &= ~__LPG_NODE_VALUE_MASK;
}


inline void __lpg_add_child(lpg_node_t *node, lpg_node_t *child)
{
    ++node->children_size;
    node->children = realloc(node->children,sizeof(lpg_node_t*)*node->children_size);
    node->children[node->children_size-1] = child;
}


inline void __lpg_node_release(lpg_node_t *node)
{
    lpg_node_t **parents = lpg_node_parents(node);

    if(parents)
        free(parents);
    
    if(node->children)
        free(node->children);
}


inline size_t lpg_node_get_parents_num(const lpg_node_t *node)
{
    static const uint16_t node_type_parents_num[] = {2, 2, 1, 2, 0, 0, 0};

    return node_type_parents_num[node->type];
}