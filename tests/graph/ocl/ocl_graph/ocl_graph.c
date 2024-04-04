#include <lockpick/test.h>
#include <lockpick/graph/ocl_graph.h>
#include <lockpick/graph/compute.h>
#include <lockpick/graph/types/uint.h>
#include <malloc.h>

#define __LPG_TEST_OCL_GRAPH_MAX_GRAPH_NODES 100000


void __test_ocl_graph_compute(size_t in_width, size_t out_width)
{
    lpg_graph_t *graph = lpg_graph_create("test",in_width,out_width,__LPG_TEST_OCL_GRAPH_MAX_GRAPH_NODES);
    size_t operand_width = in_width/2;
    lpg_uint_t *uint_a = lpg_uint_allocate_as_buffer_view(graph,graph->inputs,operand_width);
    lpg_uint_t *uint_b = lpg_uint_allocate_as_buffer_view(graph,graph->inputs+operand_width,operand_width);
    lpg_uint_t *uint_res = lpg_uint_allocate_as_buffer_view(graph,graph->outputs,out_width);
    lpg_uint_mul(uint_a,uint_b,uint_res);

    lpg_ocl_graph_t *ocl_graph = lpg_ocl_graph_create(graph,true);

    size_t nodes_num = ocl_graph->nodes_num;

    lpg_uint_assign_from_rand(uint_a);
    lpg_uint_assign_from_rand(uint_b);

    lpg_graph_compute(graph);

    bool *true_computed_out = (bool*)malloc(out_width*sizeof(bool));
    for(size_t out_node_i = 0; out_node_i < out_width; ++out_node_i)
        true_computed_out[out_node_i] = lpg_node_value(graph->outputs[out_node_i]);
    
    bool ocl_graph_computed_out = (bool*)malloc(nodes_num*sizeof(bool));
    for(size_t node_i = 0; node_i < nodes_num; ++node_i)
    {
        uint16_t parents_num = lpg_node_packed_get_parents_num(&ocl_graph->sorted_nodes[node_i]);
    }
}


void lp_test_ocl_graph()
{
    
}
