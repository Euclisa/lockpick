#include <lockpick/graph/tsort.h>
#include <lockpick/graph/traverse.h>
#include <lockpick/graph/ocl_graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/htable.h>
#include <lockpick/math.h>
#include <lockpick/vector.h>
#include <malloc.h>
#include <string.h>


void __lpg_ocl_graph_tsort_packed(lpg_ocl_graph_t *ocl_graph, bool gen_reverse_index)
{
    affirm_nullptr(ocl_graph,"graph");
    affirmf_debug(ocl_graph->graph,"Graph field of ocl graph instance must be set to this point");

    lpg_graph_t *graph = ocl_graph->graph;

    __tsort_init_state_t init_state;

    size_t const_nodes_init_size = (1 << INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE)*sizeof(lpg_node_t*);
    init_state.const_nodes = (lpg_node_t**)malloc(const_nodes_init_size);
    affirm_bad_malloc(init_state.const_nodes,"zero layer nodes array",const_nodes_init_size);

    init_state.nodes_count = init_state.const_nodes_count = 0;

    lpg_graph_traverse_once(graph,__lpg_tsort_init_state_cb,&init_state);

    ocl_graph->nodes_num = init_state.nodes_count;

    size_t result_size = init_state.nodes_count*sizeof(lpg_node_packed_t);
    lpg_node_packed_t *result = (lpg_node_packed_t*)malloc(result_size);
    affirm_bad_malloc(result,"result sorted nodes array",result_size);

    ocl_graph->index_map = lp_htable_create_el_num(
            init_state.nodes_count,
            sizeof(__lpg_ocl_graph_index_map_entry_t),
            lp_htable_cast_hsh(__lpg_ocl_graph_index_map_hsh),
            lp_htable_cast_eq(__lpg_ocl_graph_index_map_eq));
    
    if(gen_reverse_index)
        ocl_graph->rev_index_map = lp_htable_create_el_num(
            init_state.nodes_count,
            sizeof(__lpg_ocl_graph_index_map_entry_t),
            lp_htable_cast_hsh(__lpg_ocl_graph_rev_index_map_hsh),
            lp_htable_cast_eq(__lpg_ocl_graph_index_map_eq));
    else
        ocl_graph->rev_index_map = NULL;
    
    size_t zero_layer_size = init_state.const_nodes_count+graph->inputs_size;
    size_t init_orphaned_capacity = zero_layer_size*2;
    // Nodes that do not have unprocessed parents anymore
    lp_vector_t *orphaned = lp_vector_create(init_orphaned_capacity,sizeof(lpg_node_t*));

    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
    {
        result[in_node_i] = __lpg_node_packed_from_input_node(graph->inputs[in_node_i]);
        lp_vector_push_back(orphaned,&graph->inputs[in_node_i]);
        __lpg_ocl_graph_index_map_insert(ocl_graph,graph->inputs[in_node_i],in_node_i);
        if(gen_reverse_index)
            __lpg_ocl_graph_rev_index_map_insert(ocl_graph,graph->inputs[in_node_i],in_node_i);
    }
    
    for(size_t const_node_i = 0; const_node_i < init_state.const_nodes_count; ++const_node_i)
    {
        result[graph->inputs_size+const_node_i] = __lpg_node_packed_from_const_node(init_state.const_nodes[const_node_i]);
        lp_vector_push_back(orphaned,&init_state.const_nodes[const_node_i]);
        __lpg_ocl_graph_index_map_insert(ocl_graph,init_state.const_nodes[const_node_i],graph->inputs_size+const_node_i);
        if(gen_reverse_index)
            __lpg_ocl_graph_rev_index_map_insert(ocl_graph,init_state.const_nodes[const_node_i],graph->inputs_size+const_node_i);
    }
    free(init_state.const_nodes);

    lp_htable_t *visited = lp_htable_create_el_num(
            init_state.nodes_count,
            sizeof(lpg_node_t*),
            lp_htable_cast_hsh(__lpg_graph_nodes_hsh),
            lp_htable_cast_eq(__lpg_graph_nodes_eq));

    size_t curr_node_i = zero_layer_size;
    while(!lp_vector_empty(orphaned))
    {
        lpg_node_t *curr_node = *(lpg_node_t**)lp_vector_back(orphaned);
        size_t children_num = lpg_node_get_children_num(curr_node);
        lp_vector_pop_back(orphaned);
        for(size_t child_i = 0; child_i < children_num; ++child_i)
        {
            lpg_node_t *child_node = lp_vector_at_type(curr_node->children,child_i,lpg_node_t*);
            size_t child_parents_num = lpg_node_get_parents_num(child_node);
            affirmf_debug(child_parents_num > 0,"Parents num of a child node must always be greater than zero");
            
            if(child_parents_num == 1 || !lp_htable_insert(visited,&child_node))
            {
                lp_vector_push_back(orphaned,&child_node);
                result[curr_node_i] = __lpg_node_packed_from_node(ocl_graph,child_node);

                __lpg_ocl_graph_index_map_insert(ocl_graph,child_node,curr_node_i);
                if(gen_reverse_index)
                    __lpg_ocl_graph_rev_index_map_insert(ocl_graph,child_node,curr_node_i);

                ++curr_node_i;
            }
        }
    }

    ocl_graph->sorted_nodes = result;

    lp_htable_release(visited);
    lp_vector_release(orphaned);
}
