#include <lockpick/test.h>
#include <lockpick/graph/ocl_graph.h>
#include <lockpick/graph/compute.h>
#include <lockpick/graph/types/uint.h>
#include <lockpick/bitset.h>
#include <malloc.h>
#include <stdio.h>

#define __LPG_TEST_OCL_GRAPH_MAX_GRAPH_NODES 100000


void __test_ocl_graph_compute(size_t in_width, size_t out_width)
{
    lpg_graph_t *graph = lpg_graph_create("test",in_width,out_width,__LPG_TEST_OCL_GRAPH_MAX_GRAPH_NODES);
    size_t operand_width = in_width/2;
    lpg_uint_t *uint_a = lpg_uint_allocate_as_buffer_view(graph,graph->inputs,operand_width);
    lpg_uint_t *uint_b = lpg_uint_allocate_as_buffer_view(graph,graph->inputs+operand_width,operand_width);
    lpg_uint_t *uint_res = lpg_uint_allocate_as_buffer_view(graph,graph->outputs,out_width);
    lpg_uint_mul(uint_a,uint_b,uint_res);

    lpg_uint_assign_from_rand(uint_a);
    lpg_uint_assign_from_rand(uint_b);
    lpg_graph_compute(graph);

    lpg_ocl_graph_t *ocl_graph = lpg_ocl_graph_create(graph,true);
    lp_bitset_t *input_values = lp_bitset_create(graph->inputs_size);
    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
    {
        bool in_value = lpg_node_value(graph->inputs[in_node_i]);
        lp_bitset_update(input_values,in_node_i,in_value);
    }

    lp_bitset_t *ocl_computed_output = lpg_ocl_graph_compute_host(ocl_graph,input_values);

    for(size_t out_node_i = 0; out_node_i < graph->outputs_size; ++out_node_i)
    {
        bool ocl_out_value = lp_bitset_test(ocl_computed_output,out_node_i);
        bool true_out_value = lpg_node_value(graph->outputs[out_node_i]);

        LP_TEST_ASSERT(ocl_out_value == true_out_value,
            "Output at index %zd expected: %d, got: %d. (in_width: %zd, out_width: %zd)",
            out_node_i,(uint32_t)true_out_value,(uint32_t)ocl_out_value,in_width,out_width);
    }

    lp_test_cleanup:
    lpg_graph_release(graph);
    lpg_uint_release(uint_a);
    lpg_uint_release(uint_b);
    lpg_uint_release(uint_res);
    lp_bitset_release(input_values);
    lp_bitset_release(ocl_computed_output);
    lpg_ocl_graph_release(ocl_graph);
}


void test_ocl_graph_compute()
{
    for(size_t in_width = 2; in_width <= 18; in_width += 2)
        for(size_t out_width = in_width/2; out_width <= in_width; ++out_width)
            LP_TEST_STEP_INTO(__test_ocl_graph_compute(in_width,out_width));
    
    lp_test_cleanup:
}


void lp_test_ocl_graph()
{
    LP_TEST_RUN(test_ocl_graph_compute());
}
