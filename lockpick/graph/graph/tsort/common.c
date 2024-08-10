#include <lockpick/graph/tsort.h>
#include <lockpick/affirmf.h>
#include <lockpick/math.h>


void __lpg_tsort_init_state_cb(lpg_graph_t *graph, lpg_node_t *node, bool is_input, void *args)
{
    __tsort_init_state_t *init_state = args;

    static const size_t zero_layer_nodes_init_size = (1 << INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE);

    ++init_state->nodes_count;
    
    if(!is_input && lpg_node_get_parents_num(node) == 0)
    {
        if(init_state->const_nodes_count >= zero_layer_nodes_init_size && lp_is_pow_2(init_state->const_nodes_count))
        {
            size_t new_zero_layer_nodes_size = (init_state->const_nodes_count << 1)*sizeof(lpg_node_t*);
            init_state->const_nodes = (lpg_node_t**)realloc(init_state->const_nodes,new_zero_layer_nodes_size);
            affirm_bad_malloc(init_state->const_nodes,"zero layer nodes array realloc",new_zero_layer_nodes_size);
        }
        
        init_state->const_nodes[init_state->const_nodes_count] = node;
        ++init_state->const_nodes_count;
    }
}
