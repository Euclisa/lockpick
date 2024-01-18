#ifndef _LOCKPICK_INCLUDE_GRAPH_NODE_H
#define _LOCKPICK_INCLUDE_GRAPH_NODE_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <lockpick/list.h>
#include <lockpick/define.h>
#include <lockpick/slab.h>


typedef enum lpg_node_types
{
    LPG_NODE_TYPE_AND,
    LPG_NODE_TYPE_OR,
    LPG_NODE_TYPE_NOT,
    LPG_NODE_TYPE_XOR,
    LPG_NODE_TYPE_TRUE,
    LPG_NODE_TYPE_FALSE,
    LPG_NODE_TYPE_VAR
} lpg_node_type_t;


typedef struct lpg_node
{
    lpg_node_type_t type;
    struct lpg_node **parents;
} lpg_node_t;

typedef struct lpg_node_fast
{
    lpg_node_t node;
    lpg_node_t **children;
} lpg_node_fast_t;


lp_slab_t *lpg_node_create_slab(size_t total_entries);

lpg_node_t *lpg_node_and(lp_slab_t *slab, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_or(lp_slab_t *slab, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_not(lp_slab_t *slab, lpg_node_t *a);
lpg_node_t *lpg_node_xor(lp_slab_t *slab, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_const(lp_slab_t *slab, bool value);
lpg_node_t *lpg_node_var(lp_slab_t *slab);

#endif // _LOCKPICK_INCLUDE_GRAPH_NODE_H