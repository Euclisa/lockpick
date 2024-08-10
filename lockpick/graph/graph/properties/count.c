#include <lockpick/graph/count.h>
#include <lockpick/graph/traverse.h>
#include <lockpick/affirmf.h>


/**
 * lpg_graph_nodes_count_super - counts number of nodes in super-graph
 * @graph:      pointer to the graph object
 * 
 * This function derives the number of nodes in the super-graph corresponding to the
 * given @graph object. The super-graph manages the slab allocator for nodes, while
 * sub-graphs merely reference it.
 * 
 * Since the slab allocator contains the exact number of nodes that the well-formed
 * graph consists of, the return value can be easily computed by counting the number
 * of occupied slots in the graph's slab allocator.
 * 
 * Return: The number of nodes in the super-graph. 
*/
inline size_t lpg_graph_nodes_count_super(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    return slab->__total_entries - slab->__total_free;
}


static void __lpg_graph_nodes_count_cb(lpg_graph_t *graph, lpg_node_t *node, bool is_input, void *args)
{
    _Atomic size_t *count = args;
    ++(*count);
}


/**
 * lpg_graph_nodes_count - counts graph's nodes using parallel traversal algorithm
 * @graph:      pointer to the graph object
 * 
 * This function counts the number of nodes in the given @graph, adhering to the general
 * graph paradigm. In this paradigm, input nodes are treated as terminal nodes, and the
 * traversal stops upon reaching these input nodes, even if they have unvisited parent
 * nodes (operands).
 * 
 * Uses parallel traversal algorithm.
 * 
 * Consequently, there is no alternative way to count the nodes accurately other than
 * performing a graph traversal. This function runs a traversal to determine the total
 * number of nodes in the graph, including the input nodes where the traversal terminates.
 * 
 * WARNING: Parallel traversal algorithm proved to be much slower than sequential in
 * task of counting graph nodes. Prefer usage of 'lpg_graph_nodes_count' instead.
 * 
 * Return: total number of nodes in the graph.
*/
size_t lpg_graph_nodes_count_mt(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    _Atomic size_t count = 0;
    lpg_graph_traverse_once_sync(graph,__lpg_graph_nodes_count_cb,&count);

    return count;
}


/**
 * lpg_graph_nodes_count - counts graph's nodes
 * @graph:      pointer to the graph object
 * 
 * This function counts the number of nodes in the given @graph, adhering to the general
 * graph paradigm. In this paradigm, input nodes are treated as terminal nodes, and the
 * traversal stops upon reaching these input nodes, even if they have unvisited parent
 * nodes (operands).
 * 
 * Consequently, there is no alternative way to count the nodes accurately other than
 * performing a graph traversal. This function runs a traversal to determine the total
 * number of nodes in the graph, including the input nodes where the traversal terminates.
 * 
 * Return: total number of nodes in the graph.
*/
size_t lpg_graph_nodes_count(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    _Atomic size_t count = 0;
    lpg_graph_traverse_once(graph,__lpg_graph_nodes_count_cb,&count);

    return count;
}
