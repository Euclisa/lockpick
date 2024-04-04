#ifndef _LOCKPICK_GRAPH_OCL_GRAPH_H
#define _LOCKPICK_GRAPH_OCL_GRAPH_H

#include <lockpick/graph/graph.h>
#include <lockpick/htable.h>
#include <lockpick/bitset.h>
#include <CL/opencl.h>
#include <stdbool.h>

#define LPG_OCL_GRAPH_MAX_NODES_NUM 65535

typedef struct lpg_ocl_graph lpg_ocl_graph_t;

/*
    WARNING: Do not rearange fields of this enum,
    their order is used for fast conversion from regular 'node_type'
*/
enum lpg_node_packed_types
{
    LPG_NODE_PACKED_TYPE_AND,
    LPG_NODE_PACKED_TYPE_OR,
    LPG_NODE_PACKED_TYPE_NOT,
    LPG_NODE_PACKED_TYPE_XOR,
    LPG_NODE_PACKED_TYPE_TRUE,
    LPG_NODE_PACKED_TYPE_FALSE,
    LPG_NODE_PACKED_TYPE_INPUT
};


typedef struct lpg_node_packed
{
    cl_half parents[2];
    cl_char type;

    cl_char __padd;
} __packed lpg_node_packed_t;


uint16_t lpg_node_packed_get_parents_num(const lpg_node_packed_t *node);

cl_char __lpg_node_packed_type_from_node(const lpg_node_t *node);

lpg_node_packed_t __lpg_node_packed_from_node(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node);
lpg_node_packed_t __lpg_node_packed_from_const_node(const lpg_node_t *node);
lpg_node_packed_t __lpg_node_packed_from_input_node(const lpg_node_t *node);


struct lpg_ocl_graph
{
    lpg_graph_t *graph;
    lp_htable_t *index_map,*rev_index_map;
    size_t nodes_num;
    lpg_node_packed_t *sorted_nodes;
};


lpg_ocl_graph_t *lpg_ocl_graph_create(lpg_graph_t *graph, bool gen_reverse_index);

void lpg_ocl_graph_release(lpg_ocl_graph_t *ocl_graph);


typedef struct __lpg_ocl_graph_index_map_entry
{
    lpg_node_t *node;
    uint16_t index;
} __lpg_ocl_graph_index_map_entry_t;

size_t __lpg_ocl_graph_index_map_hsh(const __lpg_ocl_graph_index_map_entry_t *entry);
bool __lpg_ocl_graph_index_map_eq(const __lpg_ocl_graph_index_map_entry_t *a, const __lpg_ocl_graph_index_map_entry_t *b);

void __lpg_ocl_graph_index_map_insert(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node, uint16_t index);
void lpg_ocl_graph_index_map_find(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node, uint16_t *result);

size_t __lpg_ocl_graph_rev_index_map_hsh(const __lpg_ocl_graph_index_map_entry_t *entry);
bool __lpg_ocl_graph_rev_index_map_eq(const __lpg_ocl_graph_index_map_entry_t *a, const __lpg_ocl_graph_index_map_entry_t *b);

void __lpg_ocl_graph_rev_index_map_insert(lpg_ocl_graph_t *ocl_graph, lpg_node_t *node, uint16_t index);
void lpg_ocl_graph_rev_index_map_find(lpg_ocl_graph_t *ocl_graph, uint16_t index, lpg_node_t **result);


void __lpg_ocl_graph_tsort_packed(lpg_ocl_graph_t *ocl_graph, bool gen_reverse_index);


lp_bitset_t *lpg_ocl_graph_compute_host(lpg_ocl_graph_t *ocl_graph, const lp_bitset_t *input_values);

#endif // _LOCKPICK_GRAPH_OCL_GRAPH_H