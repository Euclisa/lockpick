#ifndef _LOCKPICK_GRAPH_INFERENCE_PACKED_NODE_H
#define _LOCKPICK_GRAPH_INFERENCE_PACKED_NODE_H

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

#ifndef __OPENCL_C_VERSION__
#include <CL/cl.h>

typedef cl_short lpg_inference_graph_index_t;
#else
typedef short lpg_inference_graph_index_t;
#endif // __OPENCL_C_VERSION__




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
} __attribute__((packed)) lpg_node_packed_t;


#endif // _LOCKPICK_GRAPH_INFERENCE_PACKED_NODE_H
