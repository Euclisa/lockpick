#include <lockpick/graph/ocl_graph.h>
#include <lockpick/bitset.h>


lp_bitset_t *lpg_ocl_graph_compute_host(lpg_ocl_graph_t *ocl_graph, const lp_bitset_t *input_values)
{
    affirm_nullptr(ocl_graph,"ocl graph");

    lp_bitset_t *values = lp_bitset_create(ocl_graph->nodes_num);
    lp_bitset_copy(values,input_values);

    uint16_t inputs_size = ocl_graph->graph->inputs_size;
    for(uint16_t node_i = inputs_size; node_i < ocl_graph->nodes_num; ++node_i)
    {
        lpg_node_packed_t curr_node = ocl_graph->sorted_nodes[node_i];
        bool curr_node_value;

        switch(curr_node.type)
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
                errorf("Unknown type: %d",(uint32_t)curr_node.type);
        }

        lp_bitset_update(values,node_i,curr_node_value);
    }

    uint16_t outputs_size = ocl_graph->graph->outputs_size;
    lp_bitset_t *output = lp_bitset_create(outputs_size);

    for(uint16_t out_node_i = 0; out_node_i < outputs_size; ++out_node_i)
    {
        uint16_t curr_out_index;
        lpg_ocl_graph_index_map_find(ocl_graph,ocl_graph->graph->outputs[out_node_i],&curr_out_index);
        bool curr_out_value = lp_bitset_test(values,curr_out_index);
        lp_bitset_update(output,out_node_i,curr_out_value);
    }

    lp_bitset_release(values);

    return output;
}
