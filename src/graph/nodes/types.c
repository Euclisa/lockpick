#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


void __lpg_node_init_and(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    lpg_node_t **parents_ptr = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*2);
    __lpg_node_set_parents(node,parents_ptr);

    lpg_node_parents(node)[0] = a;
    lpg_node_parents(node)[1] = b;

    node->type = LPG_NODE_TYPE_AND;
}

void __lpg_node_init_or(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    lpg_node_t **parents_ptr = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*2);
    __lpg_node_set_parents(node,parents_ptr);

    lpg_node_parents(node)[0] = a;
    lpg_node_parents(node)[1] = b;

    node->type = LPG_NODE_TYPE_OR;
}

void __lpg_node_init_not(lpg_node_t *node, lpg_node_t* a)
{
    lpg_node_t **parents_ptr = (lpg_node_t**)malloc(sizeof(lpg_node_t*));
    __lpg_node_set_parents(node,parents_ptr);

    lpg_node_parents(node)[0] = a;

    node->type = LPG_NODE_TYPE_NOT;
}

void __lpg_node_init_xor(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    lpg_node_t **parents_ptr = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*2);
    __lpg_node_set_parents(node,parents_ptr);

    lpg_node_parents(node)[0] = a;
    lpg_node_parents(node)[1] = b;

    node->type = LPG_NODE_TYPE_XOR;
}

void __lpg_node_init_const(lpg_node_t *node, bool value)
{
    node->__parents_computed_value = NULL;
    node->type = value ? LPG_NODE_TYPE_TRUE : LPG_NODE_TYPE_FALSE;
}

void __lpg_node_init_var(lpg_node_t *node)
{
    node->__parents_computed_value = NULL;
    node->type = LPG_NODE_TYPE_VAR;
}


lpg_node_t *__lpg_node_alloc(lp_slab_t *slab)
{
    lpg_node_t *node;
    if(slab)
    {
        #ifdef LOCKPICK_DEBUG
        affirmf(slab->__entry_size == sizeof(lpg_node_t),
            "Got slab with invalid entry size");
        #endif // LOCKPICK_DEBUG

        node = (lpg_node_t*)lp_slab_alloc(slab);
        affirmf(node,"Failed to allocate space for node from specified slab");
    }
    else
    {
        node = (lpg_node_t*)malloc(sizeof(lpg_node_t));
        affirmf(node,"Failed to allocate space for node from memory");
    }

    return node;
}


lpg_node_t *lpg_node_and(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    lpg_node_t *node = __lpg_node_alloc(graph->slab);
    __lpg_node_init_and(node,a,b);

    return node;
}

lpg_node_t *lpg_node_or(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    lpg_node_t *node = __lpg_node_alloc(graph->slab);
    __lpg_node_init_or(node,a,b);

    return node;
}

lpg_node_t *lpg_node_not(lpg_graph_t *graph, lpg_node_t *a)
{
    lpg_node_t *node = __lpg_node_alloc(graph->slab);
    __lpg_node_init_not(node,a);

    return node;
}

lpg_node_t *lpg_node_xor(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    lpg_node_t *node = __lpg_node_alloc(graph->slab);
    __lpg_node_init_xor(node,a,b);

    return node;
}

lpg_node_t *lpg_node_const(lpg_graph_t *graph, bool value)
{
    lpg_node_t *node = __lpg_node_alloc(graph->slab);
    __lpg_node_init_const(node,value);

    return node;
}

lpg_node_t *lpg_node_var(lpg_graph_t *graph)
{
    lpg_node_t *node = __lpg_node_alloc(graph->slab);
    __lpg_node_init_var(node);

    return node;
}