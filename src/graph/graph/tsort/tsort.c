#include <lockpick/graph/tsort.h>
#include <lockpick/graph/traverse.h>
#include <lockpick/affirmf.h>
#include <lockpick/htable.h>
#include <lockpick/math.h>
#include <lockpick/vector.h>
#include <malloc.h>
#include <string.h>


/**
 * lpg_graph_tsort - sorts graph's nodes in topological order
 * @graph:          pointer to graph object
 * @sorted_nodes:   pointer on array of sorted nodes
 * 
 * This function performs topological sorting of the nodes in the provided @graph object.
 * It allocates a buffer to store the sorted nodes, with the size of the buffer equal to
 * the number of nodes in the graph.
 * 
 * At the end, @sorted_nodes is guaranteed to be pointing to an array of pointers to nodes
 * That are ordered in a way that for every node all its parents are placed before.
 * 
 * This variant does the job sequentially without using parallelism.
 * 
 * The function adheres to the general graph paradigm and treats the input nodes of the
 * graph as terminal nodes, even if they have parent nodes (operands). The traversal
 * stops upon reaching these input nodes. Consequently, it is not possible to determine
 * the nodes in the zero graph component or the total number of nodes in the @graph.
 * Therefore, this routine first traverses the graph to gather this information.
 * 
 * WARNING: This procedure assumes that all nodes have at most two parents. If you
 * decide to extend the set of operation (node) types by adding more sophisticated
 * operations, you will need to modify this routine accordingly.
*/
void lpg_graph_tsort(lpg_graph_t *graph, lpg_node_t ***sorted_nodes)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(sorted_nodes,"sorted nodes array pointer");

    __tsort_init_state_t init_state;

    size_t const_nodes_init_size = (1 << INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE)*sizeof(lpg_node_t*);
    init_state.const_nodes = (lpg_node_t**)malloc(const_nodes_init_size);
    affirm_bad_malloc(init_state.const_nodes,"zero layer nodes array",const_nodes_init_size);

    init_state.nodes_count = init_state.const_nodes_count = 0;

    lpg_graph_traverse_once(graph,__lpg_tsort_init_state_cb,&init_state);

    size_t result_size = init_state.nodes_count*sizeof(lpg_node_t*);
    lpg_node_t **result = (lpg_node_t**)malloc(result_size);
    affirm_bad_malloc(result,"result sorted nodes array",result_size);

    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
        result[in_node_i] = graph->inputs[in_node_i];
    
    for(size_t const_node_i = 0; const_node_i < init_state.const_nodes_count; ++const_node_i)
        result[graph->inputs_size+const_node_i] = init_state.const_nodes[const_node_i];
    
    free(init_state.const_nodes);

    size_t zero_layer_size = init_state.const_nodes_count+graph->inputs_size;

    lp_htable_t *visited = lp_htable_create_el_num(
            init_state.nodes_count,
            sizeof(lpg_node_t*),
            lp_htable_cast_hsh(__lpg_graph_nodes_hsh),
            lp_htable_cast_eq(__lpg_graph_nodes_eq));
    
    size_t init_orphaned_capacity = zero_layer_size*2;
    // Nodes that do not have unprocessed parents anymore
    lp_vector_t *orphaned = lp_vector_create(init_orphaned_capacity,sizeof(lpg_node_t*));

    for(size_t node_i = 0; node_i < zero_layer_size; ++node_i)
        lp_vector_push_back(orphaned,&result[node_i]);

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
                result[curr_node_i] = child_node;
                ++curr_node_i;
            }
        }
    }

    *sorted_nodes = result;

    lp_htable_release(visited);
    lp_vector_release(orphaned);
}
