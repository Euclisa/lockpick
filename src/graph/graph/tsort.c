#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/htable.h>
#include <lockpick/math.h>
#include <lockpick/vector.h>
#include <malloc.h>


typedef struct __tsort_init_state
{
    lpg_node_t **zero_layer_nodes;
    size_t nodes_count,zero_layer_nodes_count;
} __tsort_init_state_t;


#define INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE 3


void __lpg_tsort_init_state_cb(lpg_graph_t *graph, lpg_node_t *node, bool is_input, void *args)
{
    __tsort_init_state_t *init_state = args;

    static const size_t zero_layer_nodes_init_size = (1 << INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE);

    ++init_state->nodes_count;
    
    if(is_input || lpg_node_get_parents_num(node) == 0)
    {
        if(init_state->zero_layer_nodes_count >= zero_layer_nodes_init_size && lp_is_pow_2(init_state->zero_layer_nodes_count))
        {
            size_t new_zero_layer_nodes_size = (init_state->zero_layer_nodes_count << 1)*sizeof(lpg_node_t*);
            init_state->zero_layer_nodes = (lpg_node_t**)realloc(init_state->zero_layer_nodes,new_zero_layer_nodes_size);
            affirm_bad_malloc(init_state->zero_layer_nodes,"zero layer nodes array realloc",new_zero_layer_nodes_size);
        }
        
        init_state->zero_layer_nodes[init_state->zero_layer_nodes_count] = node;
        ++init_state->zero_layer_nodes_count;
    }
}

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

    __tsort_init_state_t init_state;

    size_t zero_layer_nodes_init_size = (1 << INIT_ZERO_LAYER_NODES_LOG_ARR_SIZE)*sizeof(lpg_node_t*);
    init_state.zero_layer_nodes = (lpg_node_t**)malloc(zero_layer_nodes_init_size);
    affirm_bad_malloc(init_state.zero_layer_nodes,"zero layer nodes array",zero_layer_nodes_init_size);

    init_state.nodes_count = init_state.zero_layer_nodes_count = 0;

    lpg_graph_traverse(graph,__lpg_tsort_init_state_cb,&init_state,NULL,NULL);

    size_t sorted_nodes_size = (init_state.nodes_count)*sizeof(lpg_node_t*);
    init_state.zero_layer_nodes = (lpg_node_t**)realloc(init_state.zero_layer_nodes,sorted_nodes_size);
    affirm_bad_malloc(init_state.zero_layer_nodes,"topologicaly sorted nodes array realloc",sorted_nodes_size);

    lp_htable_t *visited = lp_htable_create_el_num(
            init_state.nodes_count,
            sizeof(lpg_node_t*),
            (size_t (*)(const void *))__lpg_graph_nodes_hsh,
            (bool (*)(const void *,const void *))__lpg_graph_nodes_eq);
    
    size_t init_orphaned_capacity = init_state.zero_layer_nodes_count*2;
    // Nodes that do not have unprocessed parents anymore
    lp_vector_t *orphaned = lp_vector_create(init_orphaned_capacity,sizeof(lpg_node_t*));

    for(size_t node_i = 0; node_i < init_state.zero_layer_nodes_count; ++node_i)
        lp_vector_push_back(orphaned,&init_state.zero_layer_nodes[node_i]);
    
    size_t curr_node_i = init_state.zero_layer_nodes_count;
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
                init_state.zero_layer_nodes[curr_node_i] = child_node;
                ++curr_node_i;
            }
        }
    }

    *sorted_nodes = init_state.zero_layer_nodes;

    lp_htable_release(visited);
    lp_vector_release(orphaned);
}
