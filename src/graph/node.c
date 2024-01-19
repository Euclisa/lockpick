#include <lockpick/graph/node.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>


lp_slab_t *lpg_node_create_slab(size_t total_entries)
{
    lp_slab_t *slab = lp_slab_create(total_entries,sizeof(lpg_node_t));
    affirmf(slab,"Failed to create slab if for %ld entries",total_entries);

    return slab;
}


void __lpg_node_init_and(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    node->parents = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*2);
    node->parents[0] = a;
    node->parents[1] = b;
    node->type = LPG_NODE_TYPE_AND;
}

void __lpg_node_init_or(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    node->parents = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*2);
    node->parents[0] = a;
    node->parents[1] = b;
    node->type = LPG_NODE_TYPE_OR;
}

void __lpg_node_init_not(lpg_node_t *node, lpg_node_t* a)
{
    node->parents = (lpg_node_t**)malloc(sizeof(lpg_node_t*));
    node->parents[0] = a;
    node->type = LPG_NODE_TYPE_NOT;
}

void __lpg_node_init_xor(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    node->parents = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*2);
    node->parents[0] = a;
    node->parents[1] = b;
    node->type = LPG_NODE_TYPE_XOR;
}

void __lpg_node_init_const(lpg_node_t *node, bool value)
{
    node->parents = NULL;
    node->type = value ? LPG_NODE_TYPE_TRUE : LPG_NODE_TYPE_FALSE;
}

void __lpg_node_init_var(lpg_node_t *node)
{
    node->parents = NULL;
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


lpg_node_t *lpg_node_and(lp_slab_t *slab, lpg_node_t *a, lpg_node_t *b)
{
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_and(node,a,b);

    return node;
}

lpg_node_t *lpg_node_or(lp_slab_t *slab, lpg_node_t *a, lpg_node_t *b)
{
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_or(node,a,b);

    return node;
}

lpg_node_t *lpg_node_not(lp_slab_t *slab, lpg_node_t *a)
{
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_not(node,a);

    return node;
}

lpg_node_t *lpg_node_xor(lp_slab_t *slab, lpg_node_t *a, lpg_node_t *b)
{
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_xor(node,a,b);

    return node;
}

lpg_node_t *lpg_node_const(lp_slab_t *slab, bool value)
{
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_const(node,value);

    return node;
}

lpg_node_t *lpg_node_var(lp_slab_t *slab)
{
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_var(node);

    return node;
}