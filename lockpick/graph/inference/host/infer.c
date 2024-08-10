#include <lockpick/graph/inference/host/infer.h>
#include <lockpick/bitset.h>


lp_bitset_t *lpg_inference_graph_infer_host(lpg_inference_graph_t *inference_graph, const lp_bitset_t *input_values)
{
    affirm_nullptr(inference_graph,"ocl graph");

    lp_bitset_t *values = lp_bitset_create(inference_graph->nodes_num);
    lp_bitset_copy(values,input_values);

    uint16_t outputs_size = inference_graph->graph->outputs_size;
    lp_bitset_t *output = lp_bitset_create(outputs_size);

    uint16_t inputs_size = inference_graph->graph->inputs_size;
    lpg_inference_graph_index_t curr_out_i = 0;
    for(uint16_t node_i = inputs_size; node_i < inference_graph->nodes_num; ++node_i)
    {
        lpg_node_packed_t curr_node = inference_graph->sorted_nodes[node_i];
        cl_char type = lpg_node_packed_type(&curr_node);
        bool curr_node_value;

        switch(type)
        {
            case LPG_NODE_PACKED_TYPE_AND:
                curr_node_value =
                    lp_bitset_test(values,curr_node.parents[0]) && lp_bitset_test(values,curr_node.parents[1]);
                break;
            
            case LPG_NODE_PACKED_TYPE_OR:
                curr_node_value =
                    lp_bitset_test(values,curr_node.parents[0]) || lp_bitset_test(values,curr_node.parents[1]);
                break;
            
            case LPG_NODE_PACKED_TYPE_NOT:
                curr_node_value =
                    !lp_bitset_test(values,curr_node.parents[0]);
                break;
            
            case LPG_NODE_PACKED_TYPE_XOR:
                curr_node_value =
                    lp_bitset_test(values,curr_node.parents[0]) ^ lp_bitset_test(values,curr_node.parents[1]);
                break;
            
            case LPG_NODE_PACKED_TYPE_TRUE:
                curr_node_value = true;
                break;
            
            case LPG_NODE_PACKED_TYPE_FALSE:
                curr_node_value = false;
                break;

            default:
                errorf("Unknown type: %d",(uint32_t)type);
        }

        lp_bitset_update(values,node_i,curr_node_value);

        if(lpg_node_packed_output(&curr_node) != LPG_NODE_PACKED_NOT_OUTPUT)
        {
            lp_bitset_update(output,curr_out_i,curr_node_value);
            ++curr_out_i;
        }
    }

    lp_bitset_release(values);

    return output;
}
