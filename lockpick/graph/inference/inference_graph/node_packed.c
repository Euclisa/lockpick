#include <lockpick/graph/inference/inference_graph.h>

#define __LPG_NODE_PACKED_OUTPUT_MASK_OFFSET (__builtin_ffs(__LPG_NODE_PACKED_OUTPUT_MASK)-1)
#define __LPG_NODE_PACKED_TYPE_MASK_OFFSET (__builtin_ffs(__LPG_NODE_PACKED_TYPE_MASK)-1)


cl_char lpg_node_packed_type(const lpg_node_packed_t *node)
{
    return (node->__output_type & __LPG_NODE_PACKED_TYPE_MASK) >> __LPG_NODE_PACKED_TYPE_MASK_OFFSET;
}

void __lpg_node_packed_set_type(lpg_node_packed_t *node, cl_char type)
{
    lpg_inference_graph_index_t reset = node->__output_type & (~__LPG_NODE_PACKED_TYPE_MASK);
    lpg_inference_graph_index_t assign = (((lpg_inference_graph_index_t)type) << __LPG_NODE_PACKED_TYPE_MASK_OFFSET) & __LPG_NODE_PACKED_TYPE_MASK;
    node->__output_type = reset | assign;
}


lpg_inference_graph_index_t lpg_node_packed_output(const lpg_node_packed_t *node)
{
    return (node->__output_type & __LPG_NODE_PACKED_OUTPUT_MASK) >> __LPG_NODE_PACKED_OUTPUT_MASK_OFFSET; 
}

void __lpg_node_packed_set_output(lpg_node_packed_t *node, lpg_inference_graph_index_t output)
{
    lpg_inference_graph_index_t reset = node->__output_type & (~__LPG_NODE_PACKED_OUTPUT_MASK);
    lpg_inference_graph_index_t assign = (output << __LPG_NODE_PACKED_OUTPUT_MASK_OFFSET) & __LPG_NODE_PACKED_OUTPUT_MASK;
    node->__output_type = reset | assign;
}


uint16_t lpg_node_packed_get_parents_num(const lpg_node_packed_t *node)
{
    static const uint16_t node_type_parents_num[] = {2, 2, 1, 2, 0, 0, 0};

    cl_char type = lpg_node_packed_type(node);
    return node_type_parents_num[type];
}


cl_char __lpg_node_packed_type_from_node(const lpg_node_t *node)
{
    cl_char packed_type = 0;
    switch(node->type)
    {
        case LPG_NODE_TYPE_AND:
        case LPG_NODE_TYPE_OR:
        case LPG_NODE_TYPE_NOT:
        case LPG_NODE_TYPE_XOR:
            packed_type = (cl_char)node->type;
            break;
        
        case LPG_NODE_TYPE_CONST:
            if(lpg_node_value(node))
                packed_type = (cl_char)LPG_NODE_PACKED_TYPE_TRUE;
            else
                packed_type = (cl_char)LPG_NODE_PACKED_TYPE_FALSE;
            break;
    }

    return packed_type;
}


inline lpg_node_packed_t __lpg_node_packed_from_node(lpg_inference_graph_t *inference_graph, lpg_node_t *node, lpg_inference_graph_index_t output_index)
{
    lpg_node_packed_t packed_node;

    size_t parents_num = lpg_node_get_parents_num(node);
    lpg_node_t **parents = lpg_node_parents(node);
    for(size_t parent_i = 0; parent_i < parents_num; ++parent_i)
    {
        lpg_inference_graph_index_t packed_parent_i;
        lpg_inference_graph_index_map_find(inference_graph,parents[parent_i],&packed_parent_i);
        packed_node.parents[parent_i] = packed_parent_i;
    }
    cl_char type = __lpg_node_packed_type_from_node(node);
    __lpg_node_packed_set_type(&packed_node,type);

    __lpg_node_packed_set_output(&packed_node,output_index);

    return packed_node;
}


inline lpg_node_packed_t __lpg_node_packed_from_const_node(const lpg_node_t *node, lpg_inference_graph_index_t output_index)
{
    lpg_node_packed_t packed_node;

    cl_char type = __lpg_node_packed_type_from_node(node);

    affirmf_debug(type == LPG_NODE_PACKED_TYPE_TRUE || type == LPG_NODE_PACKED_TYPE_FALSE,
            "Expected node of terminal type but type %d was given",(uint32_t)type);
    
    __lpg_node_packed_set_type(&packed_node,type);
    __lpg_node_packed_set_output(&packed_node,output_index);

    return packed_node;
}


inline lpg_node_packed_t __lpg_node_packed_from_input_node(const lpg_node_t *node, lpg_inference_graph_index_t output_index)
{
    lpg_node_packed_t packed_node;
    __lpg_node_packed_set_type(&packed_node,LPG_NODE_PACKED_TYPE_INPUT);
    __lpg_node_packed_set_output(&packed_node,output_index);

    return packed_node;
}
