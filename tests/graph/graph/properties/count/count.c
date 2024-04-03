#include <lockpick/test.h>
#include <lockpick/graph/graph.h>
#include <lockpick/graph/types/uint.h>
#include <stdio.h>

#define __LPG_TEST_COUNT_MAX_GRAPH_NODES 100000



void __test_graph_count_nodes(size_t in_width, size_t out_width)
{
    lpg_graph_t *graph = lpg_graph_create("test",in_width,out_width,__LPG_TEST_COUNT_MAX_GRAPH_NODES);
    lpg_uint_t *uint_a = lpg_uint_allocate_as_buffer_view(graph,graph->inputs,in_width/2);
    lpg_uint_t *uint_b = lpg_uint_allocate_as_buffer_view(graph,graph->inputs+in_width/2,in_width/2);
    lpg_uint_t *uint_res = lpg_uint_allocate_as_buffer_view(graph,graph->outputs,out_width);
    lpg_uint_mul(uint_a,uint_b,uint_res);

    size_t nodes_num_true = lpg_graph_nodes_count(graph);
    size_t node_num_test = lpg_graph_nodes_count_mt(graph);

    LP_TEST_ASSERT(nodes_num_true == node_num_test,
        "For in_width: %zd, out_width: %zd, expected: %zd, got: %zd",in_width,out_width,nodes_num_true,node_num_test);
    
    lp_test_cleanup:
    lpg_graph_release(graph);
    lpg_uint_release(uint_a);
    lpg_uint_release(uint_b);
    lpg_uint_release(uint_res);
}


void lp_test_graph_count()
{
    for(size_t in_width = 2; in_width <= 18; in_width += 2)
        for(size_t out_width = 1; out_width <= in_width; ++out_width)
            LP_TEST_STEP_INTO(__test_graph_count_nodes(in_width,out_width));
    
    lp_test_cleanup:
}