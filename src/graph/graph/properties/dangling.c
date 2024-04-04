#include <lockpick/graph/count.h>
#include <lockpick/container_of.h>
#include <lockpick/affirmf.h>
#include <lockpick/rb_tree.h>
#include <lockpick/htable.h>


typedef struct __lpg_dangling_slab_callback_args
{
    lp_htable_t *dangling_black_list;
    size_t counter;
} __lpg_dangling_slab_callback_args_t;


static inline void __lpg_graph_count_dangling_slab_callback(void *entry_ptr, void *args)
{
    lpg_node_t *node = (lpg_node_t*)entry_ptr;
    __lpg_dangling_slab_callback_args_t *args_struct = (__lpg_dangling_slab_callback_args_t*)args;

    if(lpg_node_get_children_num(node) == 0)
    {
        if(!lp_htable_find(args_struct->dangling_black_list,&node,NULL))
            ++args_struct->counter;
    }
}


/**
 * lpg_graph_count_dangling_nodes - counts dangling nodes in a graph
 * @graph:      pointer to the graph object
 * 
 * Efficiently counts number of nodes that do not have children
 * and are neither input nor output nodes. Such nodes can't be reached
 * by traversal algorithms and may be considered lost.
 * 
 * Function intended to track ill-formation of graphs, which contain
 * such redudant elements.
 * 
 * The counting process is performed efficiently by iterating over all nodes within the
 * graph's slab allocator, rather than traversing the graph structure itself.
 * 
 * Returns: number of dangling nodes
*/
size_t lpg_graph_count_dangling_nodes(lpg_graph_t *graph)
{
    affirmf(graph,"Expected valid graph pointer but null was given");

    size_t total_entries = graph->inputs_size+graph->outputs_size;
    lp_htable_t *dangling_black_list = lp_htable_create_el_num(
            MAX(1,total_entries),
            sizeof(lpg_node_t*),
            lp_htable_cast_hsh(__lpg_graph_nodes_hsh),
            lp_htable_cast_eq(__lpg_graph_nodes_eq));
    
    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
        lp_htable_insert(dangling_black_list,&graph->inputs[in_node_i]);

    for(size_t out_node_i = 0; out_node_i < graph->outputs_size; ++out_node_i)
        lp_htable_insert(dangling_black_list,&graph->outputs[out_node_i]);

    __lpg_dangling_slab_callback_args_t args;
    args.counter = 0;
    args.dangling_black_list = dangling_black_list;

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lp_slab_exec(slab,__lpg_graph_count_dangling_slab_callback,&args);

    lp_htable_release(dangling_black_list);

    return args.counter;
}