#ifndef _LOCKPICK_GRAPH_GRAPH_H
#define _LOCKPICK_GRAPH_GRAPH_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <lockpick/list.h>
#include <lockpick/define.h>
#include <lockpick/slab.h>


typedef struct lpg_graph lpg_graph_t;

typedef enum lpg_node_types
{
    LPG_NODE_TYPE_AND,
    LPG_NODE_TYPE_OR,
    LPG_NODE_TYPE_NOT,
    LPG_NODE_TYPE_XOR,
    LPG_NODE_TYPE_VAR
} lpg_node_type_t;

extern const uint16_t lpg_node_type_operands_num[];


#define __LPG_NODE_PARENTS_MASK ((uintptr_t)(~0b11))
#define __LPG_NODE_COMPUTED_MASK ((uintptr_t)(0b10))
#define __LPG_NODE_VALUE_MASK ((uintptr_t)(0b1))


typedef struct lpg_node
{
    lpg_node_type_t type;
    uintptr_t __parents_computed_value;
    struct lpg_node **children;
    uint32_t children_size;
} __aligned(4) lpg_node_t;


lpg_node_t **lpg_node_parents(const lpg_node_t *node);
void __lpg_node_set_parents(lpg_node_t *node, lpg_node_t **parents);

bool lpg_node_computed(const lpg_node_t *node);
void __lpg_node_set_computed(lpg_node_t *node, bool computed);

bool lpg_node_value(const lpg_node_t *node);
void __lpg_node_set_value(lpg_node_t *node, bool value);

bool lpg_node_validate_fetch(const lpg_graph_t *graph, const lpg_node_t *node);

lpg_node_t *lpg_node_and(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_or(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_not(lpg_graph_t *graph, lpg_node_t *a);
lpg_node_t *lpg_node_xor(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b);
lpg_node_t *lpg_node_const(lpg_graph_t *graph, bool value);
lpg_node_t *lpg_node_var(lpg_graph_t *graph);

void lpg_node_compute(lpg_node_t *node);


struct lpg_graph
{
    char *name;
    lp_slab_t *slab;
    lpg_node_t **inputs;
    size_t inputs_size;
    lpg_node_t **outputs;
    size_t outputs_size;
};

lpg_graph_t *lpg_graph_create(const char *name, size_t inputs_size, size_t outputs_size, size_t max_nodes);

void lpg_graph_release(lpg_graph_t *graph);

void lpg_graph_reset(lpg_graph_t *graph);
void lpg_graph_compute(lpg_graph_t *graph);

#endif // _LOCKPICK_GRAPH_GRAPH_H