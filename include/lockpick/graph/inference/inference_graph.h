#ifndef _LOCKPICK_GRAPH_INFERENCE_INFERENCE_GRAPH_H
#define _LOCKPICK_GRAPH_INFERENCE_INFERENCE_GRAPH_H

#include <lockpick/graph/graph.h>
#include <lockpick/htable.h>
#include <lockpick/bitset.h>
#include <CL/opencl.h>
#include <stdbool.h>

#define LPG_INFERENCE_GRAPH_MAX_NODES_NUM 65536
#define LPG_INFERENCE_GRAPH_MAX_OUTPUTS_NUM 8191

typedef struct lpg_inference_graph lpg_inference_graph_t;

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


#define __LPG_NODE_PACKED_TYPE_MASK     0b0000000000000111
#define __LPG_NODE_PACKED_OUTPUT_MASK   0b1111111111111000

#define LPG_NODE_PACKED_NOT_OUTPUT      0b0001111111111111

typedef cl_half lpg_inference_graph_index_t;


/**
 * lpg_node_packed - computational graph node object specialized for efficient inference
 * @parents:        arrary of indices containing up to 2 operands (parents)
 * @__output_type:  index of this node inside outputs buffer and type of operation
 * 
 * This specialized node structure is designed for efficient inference of the corresponding computational graph,
 * which is stored as an array of such nodes sorted in topological order.
 * 
 * The @parents array contains the indices of the operand nodes within the topologically sorted array. The number
 * of parents can be efficiently derived from the node's type at runtime.
 * 
 * The @__output_type word is a packed value that contains the index of the corresponding lpg_node_t within the
 * regular lpg_graph_t outputs buffer, as well as the node's operation type.
 * 
 * If a node does not belong to the outputs buffer, the output index value is set to LPG_NODE_PACKED_NOT_OUTPUT.
 * Otherwise, the index value is less than LPG_INFERENCE_GRAPH_MAX_OUTPUTS_NUM.
 * 
 * Among the common types of node operations, there is a special LPG_NODE_PACKED_TYPE_INPUT type, which indicates
 * that the current node is an input node and its value is a variable. This is appropriate because the
 * lpg_node_packed structure does not possess a value of its own, and we cannot assign a general type like
 * LPG_NODE_PACKED_TYPE_CONST. Since input nodes are neither true nor false, a special type is introduced for
 * such nodes.
*/
typedef struct lpg_node_packed
{
    lpg_inference_graph_index_t parents[2];
    lpg_inference_graph_index_t __output_type;
} __packed lpg_node_packed_t;

cl_char lpg_node_packed_type(const lpg_node_packed_t *node);
void __lpg_node_packed_set_type(lpg_node_packed_t *node, cl_char type);

lpg_inference_graph_index_t lpg_node_packed_output(const lpg_node_packed_t *node);
void __lpg_node_packed_set_output(lpg_node_packed_t *node, lpg_inference_graph_index_t output);

uint16_t lpg_node_packed_get_parents_num(const lpg_node_packed_t *node);

cl_char __lpg_node_packed_type_from_node(const lpg_node_t *node);

lpg_node_packed_t __lpg_node_packed_from_node(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t output_index);
lpg_node_packed_t __lpg_node_packed_from_const_node(const lpg_node_t *node, lpg_inference_graph_index_t output_index);
lpg_node_packed_t __lpg_node_packed_from_input_node(const lpg_node_t *node, lpg_inference_graph_index_t output_index);

/**
 * lpg_inference_graph - interface graph structure bridging general and efficient graph representations
 * @graph:          pointer to general-purpose graph object
 * @index_map:      map between nodes and their indices within the topologically sorted array
 * @inv_index_map:  map between indices within the topologically sorted array and their corresponding node structures
 * @sorted_node:    topologically sorted array of packed nodes
 * 
 * This structure serves as an interface between the optimized graph inference engine and the general-purpose
 * graph object.
 * 
 * The general-purpose lpg_graph_t object is not well-suited for efficient evaluation of its nodes.
 * Therefore, user needs to encode it into a more suitable form – a topologically sorted array of specialized
 * packed nodes. This way, the engine can iterate over all nodes sequentially, evaluating the value of each node
 * based only on the previous nodes.
 * 
 * During creation, this structure encodes the nodes of the @graph into the topologically sorted array of packed
 * nodes @sorted_nodes. It must traverse the @graph to gather information about its structure and count the
 * @nodes_num. While populating the array, it saves the correspondence between nodes and their indices within
 * @sorted_nodes and, on demand, can build the inverse map @inv_index_map.
 * 
 * Due to memory efficiency concerns, this struct applies relatively strict constraints on the size of the @graph:
 * the number of nodes inside the graph cannot exceed LPG_INFERENCE_GRAPH_MAX_NODES_NUM; the number of output nodes
 * cannot exceed LPG_INFERENCE_GRAPH_MAX_OUTPUTS_NUM
 * 
 * The engines are expected to compute the requested information only for the @graph's output nodes, provided with
 * the @sorted_nodes array.
*/
struct lpg_inference_graph
{
    lpg_graph_t *graph;
    lp_htable_t *index_map,*inv_index_map;
    size_t nodes_num;
    lpg_node_packed_t *sorted_nodes;
};

lpg_inference_graph_t *lpg_inference_graph_create(lpg_graph_t *graph, bool gen_inverse_index);

void lpg_inference_graph_release(lpg_inference_graph_t *inference_graph);


typedef struct __lpg_inference_graph_index_map_entry
{
    lpg_node_t *node;
    lpg_inference_graph_index_t index;
} __lpg_inference_graph_index_map_entry_t;

size_t __lpg_inference_graph_index_map_hsh(const __lpg_inference_graph_index_map_entry_t *entry);
bool __lpg_inference_graph_index_map_eq(const __lpg_inference_graph_index_map_entry_t *a, const __lpg_inference_graph_index_map_entry_t *b);

void __lpg_inference_graph_index_map_insert(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t index);
void lpg_inference_graph_index_map_find(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t *result);

size_t __lpg_inference_graph_inv_index_map_hsh(const __lpg_inference_graph_index_map_entry_t *entry);
bool __lpg_inference_graph_inv_index_map_eq(const __lpg_inference_graph_index_map_entry_t *a, const __lpg_inference_graph_index_map_entry_t *b);

void __lpg_inference_graph_inv_index_map_insert(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t index);
void lpg_inference_graph_inv_index_map_find(lpg_inference_graph_t *inference_graph, lpg_inference_graph_index_t index, lpg_node_t **result);


void __lpg_inference_graph_tsort_packed(lpg_inference_graph_t *inference_graph, bool gen_inverse_index);

#endif // _LOCKPICK_GRAPH_INFERENCE_INFERENCE_GRAPH_H