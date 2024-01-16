#ifndef _LOCKPICK_INCLUDE_GRAPH_NODE_H
#define _LOCKPICK_INCLUDE_GRAPH_NODE_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <lockpick/list.h>


typedef enum lpg_node_types
{
    LP_NODE_TYPE_AND,
    LP_NODE_TYPE_OR,
    LP_NODE_TYPE_NOT,
    LP_NODE_TYPE_XOR,
    LP_NODE_TYPE_CONST,
    LP_NODE_TYPE_VAR
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


/**
 * __lpg_slab_fb_list_t - list type for slab free blocks tracking
*/
typedef struct __lpg_slab_fb_list
{
    lp_list_t __node;
    uint64_t __block_size;
    void *__start;
} __lpg_slab_fb_list_t;


typedef struct lpg_graph_slab
{
    lpg_node_t *__buffer;
    __lpg_slab_fb_list_t *__fb_head;
} lpg_graph_slab_t;

typedef struct lpg_graph_slab_fast
{
    lpg_node_fast_t *__buffer;
    __lpg_slab_fb_list_t *__fb_head;
} lpg_graph_slab_fast_t;


lpg_graph_slab_t *lpg_create_graph_slab(size_t size);

lpg_node_t *lpg_nalloc(lpg_graph_slab_t *slab);

void lpg_node_init_and(lpg_graph_slab_t *slab, lpg_node_t *node);
void lpg_node_init_or(lpg_node_t *node);
void lpg_node_init_not(lpg_node_t *node);
void lpg_node_init_xor(lpg_node_t *node);
void lpg_node_init_const(lpg_node_t *node);
void lpg_node_init_var(lpg_node_t *node);

#endif // _LOCKPICK_INCLUDE_GRAPH_NODE_H