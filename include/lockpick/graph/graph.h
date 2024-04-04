#ifndef _LOCKPICK_GRAPH_GRAPH_H
#define _LOCKPICK_GRAPH_GRAPH_H

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <lockpick/dlist.h>
#include <lockpick/define.h>
#include <lockpick/slab.h>
#include <lockpick/vector.h>


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


/**
 * lpg_node - basic general-purpose node structure for computational graphs in lpg_graph
 * @type:               underlying type of node operation
 * @__parents_value:    array of operands (parents) and value
 * @children:           array of dependend nodes (children)
 * 
 * This is a general-purpose node structure designed to be versatile for effective graph
 * manipulations, yet minimalistic. It is not intended to be optimized for fast graph
 * evaluation or maximum storage efficiency. For those purposes, users should employ
 * alternative node structures with appropriate conversion functions.
 * 
 * Each node represents a specific boolean operation, determined by the @type field.
 * 
 * Upon creation, a node must be initialized with its type and operands. Passing a node
 * as an operand argument when creating another node automatically records the latter as
 * a dependent (child) of the former.
 * 
 * The @value field is unspecified until explicitly assigned by a graph operation, such as
 * lpg_graph_compute, which computes the values of all nodes from inputs to outputs.
*/
typedef struct lpg_node
{
    lpg_node_type_t type;
    uintptr_t __parents_value;
    lp_vector_t *children;
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
size_t lpg_node_get_children_num(const lpg_node_t *node);

void __lpg_node_release_internals(lpg_node_t *node);


#define __LPG_GRAPH_SLAB_MASK ((uintptr_t)(~0b1))
#define __LPG_GRAPH_SUPER_MASK ((uintptr_t)(0b1))


/**
 * lpg_graph - general-purpose graph structure for computational graph manipulations
 * @name:           string with formal graph name
 * @__slab_super:   pointer to graph nodes slab allocator and 'super' flag
 * @inputs:         array of pointers to input nodes
 * @inputs_size:    number of input nodes
 * @outputs:        array of pointers to output nodes
 * @outputs_size:   number of output nodes
 * @max_node:       max nodes that graph may contain
 * 
 * This structure serves the purpose of storing and manipulating computational graph information and structure
 * in a way that allows easy access, inspection, and modification.
 * 
 * Upon creation, the user can provide a formal name for the graph, which is stored in @name.
 * 
 * The graph manages its nodes via a slab allocator, which can hold up to @max_nodes nodes. This cache-efficient
 * solution also provides a straightforward mechanism for determining if a particular node is allocated within
 * the specified graph context.
 * 
 * There are two types of graphs: super-graph and sub-graph. A super-graph possesses the slab
 * allocator, meaning that only a super-graph can release it. Any number of sub-graphs can be derived from a
 * super-graph, and all of them will share the same slab. Sub-graphs can be viewed as a subset of the super-graph.
 * 
 * Whether a graph is a super-graph or a sub-graph can be determined by probing the least significant bit of
 * @__slab_super, which holds the formal 'super' flag.
 * 
 * The graph maintains two arrays of nodes: @inputs and @outputs, with predefined sizes equal to @inputs_size
 * and @outputs_size, respectively. There is no way to initialize these arrays with a general-purpose constructor,
 * and the user should implement their own interfaces for populating these arrays.
 * 
 * One such approach is introduced in lockpick/graph/types/uint.h. The idea is to map the necessary memory regions
 * of these arrays and, with the provided API for node creation, populate these mapped regions according to a specific
 * rule. In the example above, input nodes are mapped to special uint objects that can be used to perform various
 * arithmetic operations, and the result can be stored in a result uint object whose buffer is mapped to a part of
 * the @outputs array.
*/
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

bool __lpg_graph_is_native_node(const lpg_graph_t *graph, const lpg_node_t *node);

size_t __lpg_graph_nodes_hsh(const lpg_node_t **node);
bool __lpg_graph_nodes_eq(const lpg_node_t **a, const lpg_node_t **b);

#endif // _LOCKPICK_GRAPH_GRAPH_H