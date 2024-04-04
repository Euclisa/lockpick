#include <lockpick/test.h>
#include <lockpick/graph/tsort.h>
#include <lockpick/graph/count.h>
#include <lockpick/graph/types/uint.h>
#include <lockpick/htable.h>

#define __LPG_TEST_TSORT_MAX_GRAPH_NODES 100000


bool __test_graph_tsort(size_t in_width, size_t out_width)
{
    lpg_graph_t *graph = lpg_graph_create("test",in_width,out_width,__LPG_TEST_TSORT_MAX_GRAPH_NODES);
    lpg_uint_t *uint_a = lpg_uint_allocate_as_buffer_view(graph,graph->inputs,in_width/2);
    lpg_uint_t *uint_b = lpg_uint_allocate_as_buffer_view(graph,graph->inputs+in_width/2,in_width/2);
    lpg_uint_t *uint_res = lpg_uint_allocate_as_buffer_view(graph,graph->outputs,out_width);
    lpg_uint_mul(uint_a,uint_b,uint_res);

    size_t redundant_inputs = lpg_graph_count_redundant_inputs(graph);
    affirmf(redundant_inputs == 0,"Graph with in_width: %zd, out_width: %zd has %zd redundant inputs and can't be sorted",in_width,out_width,redundant_inputs);

    size_t nodes_num = lpg_graph_nodes_count(graph);

    lpg_node_t **sorted_nodes;
    lpg_graph_tsort(graph,&sorted_nodes);

    lp_htable_t *visited = lp_htable_create_el_num(nodes_num,
            sizeof(lpg_node_t*),
            (size_t (*)(const void *))__lpg_graph_nodes_hsh,
            (bool (*)(const void *,const void *))__lpg_graph_nodes_eq);
    
    bool failed = false;
    for(size_t node_i = 0; node_i < nodes_num; ++node_i)
    {
        lpg_node_t *curr_node = sorted_nodes[node_i];
        size_t parents_num = lpg_node_get_parents_num(curr_node);
        lpg_node_t **parents = lpg_node_parents(curr_node);
        for(size_t parent_i = 0; parent_i < parents_num; ++parent_i)
        {
            if(!lp_htable_find(visited,&parents[parent_i],NULL))
            {
                failed = true;
                goto end_check;
            }
        }
        lp_htable_insert(visited,&curr_node);
    }
    end_check:

    lp_htable_release(visited);
    lpg_graph_release(graph);
    lpg_uint_release(uint_a);
    lpg_uint_release(uint_b);
    lpg_uint_release(uint_res);
    free(sorted_nodes);

    return failed;
}


void lp_test_graph_tsort()
{
    for(size_t in_width = 2; in_width <= 18; in_width += 2)
        for(size_t out_width = in_width/2; out_width <= in_width; ++out_width)
            LP_TEST_ASSERT(!__test_graph_tsort(in_width,out_width),
                "Sorting error in test %zd, in_width: %zd, out_width: %zd",
                in_width,out_width);
    
    lp_test_cleanup:
}