#ifndef _LOCKPICK_GRAPH_GRAPH_H
#define _LOCKPICK_GRAPH_GRAPH_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lockpick/dlist.h>
#include <lockpick/define.h>
#include <lockpick/slab.h>


typedef struct lpg_graph lpg_graph_t;

typedef enum lpg_node_types
{
    LPG_NODE_TYPE_AND,
    LPG_NODE_TYPE_OR,
    LPG_NODE_TYPE_NOT,
    LPG_NODE_TYPE_XOR,
    LPG_NODE_TYPE_CONST
} lpg_node_type_t;


#define __LPG_NODE_PARENTS_MASK ((uintptr_t)(~0b1))
#define __LPG_NODE_VALUE_MASK ((uintptr_t)(0b1))


typedef struct lpg_node
{
    lpg_node_type_t type;
    uintptr_t __parents_value;
    struct lpg_node **children;
    uint32_t children_size;
} __aligned(2) lpg_node_t;


lpg_node_t **lpg_node_parents(const lpg_node_t *node);
void __lpg_node_set_parents(lpg_node_t *node, lpg_node_t **parents);

bool lpg_node_value(const lpg_node_t *node);
void __lpg_node_set_value(lpg_node_t *node, bool value);

bool __lpg_graph_is_native_node(const lpg_graph_t *graph, const lpg_node_t *node);

lpg_node_t *lpg_node_and(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_or(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_not(lpg_graph_t *graph, lpg_node_t *a);
lpg_node_t *lpg_node_xor(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_const(lpg_graph_t *graph, bool value);

size_t lpg_node_get_parents_num(const lpg_node_t *node);

void __lpg_node_release(lpg_node_t *node);


#define __LPG_GRAPH_SLAB_MASK ((uintptr_t)(~0b1))
#define __LPG_GRAPH_SUPER_MASK ((uintptr_t)(0b1))

struct lpg_graph
{
    char *name;
    uintptr_t __slab_super;
    lpg_node_t **inputs;
    size_t inputs_size;
    lpg_node_t **outputs;
    size_t outputs_size;
    size_t max_nodes;
};

lp_slab_t *__lpg_graph_slab(const lpg_graph_t *graph);
void __lpg_graph_set_slab(lpg_graph_t *graph, lp_slab_t *slab);

bool lpg_graph_is_super(const lpg_graph_t *graph);
void __lpg_graph_set_super(lpg_graph_t *graph, bool super);

lpg_graph_t *lpg_graph_create(const char *name, size_t inputs_size, size_t outputs_size, size_t max_nodes);

void lpg_graph_release(lpg_graph_t *graph);
void lpg_graph_release_node(lpg_graph_t *graph, lpg_node_t *node);

typedef void (*lpg_traverse_cb_t)(lpg_graph_t *graph, lpg_node_t *node, void *args);
void lpg_graph_traverse_node(lpg_graph_t *graph, lpg_node_t *node, lpg_traverse_cb_t enter_cb, void *enter_cb_args, lpg_traverse_cb_t leave_cb, void *leave_cb_args);
void lpg_graph_traverse(lpg_graph_t *graph, lpg_traverse_cb_t enter_cb, void *enter_cb_args, lpg_traverse_cb_t leave_cb, void *leave_cb_args);

void lpg_graph_compute(lpg_graph_t *graph);

size_t lpg_graph_count_dangling_nodes(lpg_graph_t *graph);
size_t lpg_graph_nodes_count(lpg_graph_t *graph);
size_t lpg_graph_operators_count(lpg_graph_t *graph);

bool __lpg_graph_is_native_node(const lpg_graph_t *graph, const lpg_node_t *node);

#endif // _LOCKPICK_GRAPH_GRAPH_H