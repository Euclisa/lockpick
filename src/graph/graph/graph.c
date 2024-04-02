#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>
#include <lockpick/utility.h>
#include <lockpick/htable.h>
#include <lockpick/vector.h>
#include <string.h>


/**
 * __lpg_graph_slab - returns pointer on graph slab
 * @graph:      graph for which slab should be returned
 * 
 * Returns: pointer to slab associated with provided graph
*/
lp_slab_t *__lpg_graph_slab(const lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    return (lp_slab_t*)(graph->__slab_super & __LPG_GRAPH_SLAB_MASK);
}

/**
 * __lpg_graph_set_slab - associate slab with graph
 * @graph:      graph object  
 * @slab:       slab allocator to assign 
 *
 * Associates the given pre-allocated @slab instance with 
 * @graph to serve as its node memory allocator. 
 *
 * Return: None
*/
void __lpg_graph_set_slab(lpg_graph_t *graph, lp_slab_t *slab)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(slab,"graph slab");

    graph->__slab_super = (uintptr_t)slab | (graph->__slab_super & (~__LPG_GRAPH_SLAB_MASK));
}


/**
 * lpg_graph_is_super - check if graph is a super-graph   
 * @graph:      graph object
 *
 * Checks if given @graph object is a super-graph variety.
 * 
 * Super-graphs manage their own slab allocator to supply nodes 
 * memory for themselves and any child sub-graphs. Only super-graphs 
 * can free their slab.
 *
 * Sub-graphs are derived from existing super-graphs and share
 * slabs, but do not manage slab allocation/freeing themselves.
 *
 * Super-graph status affects graph destroy and slab management
 * capabilities. This method identifies the distinction.
 *
 * Return: True if @graph is a super-graph variety
*/
bool lpg_graph_is_super(const lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    return (bool)(graph->__slab_super & __LPG_GRAPH_SUPER_MASK);
}

/**
 * __lpg_graph_set_super - set super-graph status
 * @graph:      graph object
 * @super:      whether to set as super-graph 
 * 
 * Sets the internal super-graph status flag for @graph.
 * If @super is true, marks as super-graph. False marks 
 * as sub-graph.
 *
 * This affects assumptions and capabilities around slab
 * management and freeing responsibilities.
 * 
 * Return: None
*/
void __lpg_graph_set_super(lpg_graph_t *graph, bool super)
{
    affirm_nullptr(graph,"graph");
    
    if(super)
        graph->__slab_super |= __LPG_GRAPH_SUPER_MASK;
    else
        graph->__slab_super &= ~__LPG_GRAPH_SUPER_MASK;
}


/**
 * lpg_graph_create - allocate and initialize a graph   
 * @name:               string name for identifying the graph  
 * @inputs_size:        number of input nodes   
 * @outputs_size:       number of output nodes
 * @max_nodes:          maximum nodes allowed in the graph
 * 
 * Allocates and initializes a new graph object along with its
 * associated input, output, and slab allocator structures.
 * 
 * The @name provides a way to identify graphs, but uniqueness is  
 * not enforced. All node allocations must come from the slab so 
 * @max_nodes limits overall graph size.
 * 
 * The input buffer is prepopulated with constant 0 nodes. The output  
 * buffer is initialized to NULL and must be set by user after
 * assembling graph operations.
 * Typically this should be done by creating views on certain output
 * buffer chunks with 'lpg_uint_t' (graph uint) objects and passing them
 * as a result object to graph uint operators.
 * 
 * Return: Pointer to initialized empty graph 
*/
lpg_graph_t *lpg_graph_create(const char *name, size_t inputs_size, size_t outputs_size, size_t max_nodes)
{
    affirm_nullptr(name,"graph name string");

    lpg_graph_t *graph = (lpg_graph_t*)malloc(sizeof(lpg_graph_t));
    affirmf(graph,"Failed to allocate space for graph '%s'",name);

    size_t name_str_len = strlen(name);
    graph->name = (char*)malloc(name_str_len+1);
    strcpy(graph->name,name);

    lp_slab_t *slab = lp_slab_create(max_nodes,sizeof(lpg_node_t));
    affirmf(slab,"Failed to create slab for %zd nodes",max_nodes);
    __lpg_graph_set_slab(graph,slab);
    __lpg_graph_set_super(graph,true);

    graph->inputs = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*inputs_size);
    affirmf(graph->inputs,"Failed to allocate space for input %zd input nodes",inputs_size);
    graph->inputs_size = inputs_size;

    graph->outputs = (lpg_node_t**)calloc(outputs_size,sizeof(lpg_node_t*));
    affirmf(graph->outputs,"Failed to allocate space for input %zd input nodes",outputs_size);
    graph->outputs_size = outputs_size;

    for(size_t in_i = 0; in_i < inputs_size; ++in_i)
        graph->inputs[in_i] = lpg_node_const(graph,false);
    
    graph->max_nodes = max_nodes;

    return graph;
}


/**
 * __lpg_graph_release_slab_callback - slab callback for freeing nodes structures
 * @entry_ptr:      pointer to node structure
 * @args:           custom arguments (null here)
 * 
 * Return: None
*/
static inline void __lpg_graph_release_slab_callback(void *entry_ptr, void *args)
{
    lpg_node_t *node = (lpg_node_t*)entry_ptr;
    
    __lpg_node_release_internals(node);
}

/**
 * lpg_graph_release - release a graph object
 * @graph:      graph object to release
 * 
 * Releases the given graph object, its name string and
 * associated input and output node buffers.
 *
 * If @graph is a super-graph variety, its allocated slab  
 * memory is also freed. Sub-graph varieties do not handle  
 * slab freeing.
 *
 * After release, @graph and any child objects should no  
 * longer be used. A released super-graph slab should not
 * be referenced by any outstanding sub-graphs either.
 *
 * For simpler memory management, graphs should be released  
 * only after releasing all derived sub-graphs first.
 *
 * Return: None
*/
void lpg_graph_release(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    if(lpg_graph_is_super(graph))
    {
        lp_slab_t *slab = __lpg_graph_slab(graph);
        lp_slab_exec(slab,__lpg_graph_release_slab_callback,NULL);
        lp_slab_release(slab);
    }
    free(graph->name);
    free(graph->inputs);
    free(graph->outputs);
    free(graph);
}


void lpg_graph_release_node(lpg_graph_t *graph, lpg_node_t *node)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(node,"node");
    affirmf(__lpg_graph_is_native_node(graph,node),"Specified node does not belong to the given graph");

    size_t children_num = lpg_node_get_children_num(node);
    affirmf(children_num == 0,"Expected node with zero children, got: %d",children_num);

    size_t black_list_size = graph->inputs_size+graph->outputs_size;
    lp_htable_t *release_black_list = lp_htable_create_el_num(
            MAX(1,black_list_size),
            sizeof(lpg_node_t*),
            lp_htable_cast_hsh(__lpg_graph_nodes_hsh),
            lp_htable_cast_eq(__lpg_graph_nodes_eq));
    
    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
        lp_htable_insert(release_black_list,&graph->inputs[in_node_i]);

    for(size_t out_node_i = 0; out_node_i < graph->outputs_size; ++out_node_i)
        lp_htable_insert(release_black_list,&graph->outputs[out_node_i]);
    
    affirmf(!lp_htable_find(release_black_list,&node,NULL),
        "Can't release node which is either input or output");
    
    lp_vector_t *release_stack = lp_vector_create(0,sizeof(lpg_node_t*));
    lp_vector_push_back(release_stack,&node);

    lp_slab_t *slab = __lpg_graph_slab(graph);

    while(!lp_vector_empty(release_stack))
    {
        lpg_node_t *curr_node = lp_vector_back_type(release_stack,lpg_node_t*);
        lp_vector_pop_back(release_stack);

        if(lp_htable_find(release_black_list,&curr_node,NULL))
            continue;

        lpg_node_t **parents = lpg_node_parents(curr_node);
        size_t parents_num = lpg_node_get_parents_num(curr_node);

        for(size_t parent_i = 0; parent_i < parents_num; ++parent_i)
        {
            lpg_node_t *parent = parents[parent_i];
            size_t parent_children_num = lpg_node_get_children_num(parent);
            size_t curr_node_parent_i = 0;
            for(; curr_node_parent_i < parent_children_num; ++curr_node_parent_i)
            {
                if(lp_vector_at_type(parent->children,curr_node_parent_i,lpg_node_t*) == curr_node)
                    break;
            }

            affirmf_debug(curr_node_parent_i < parent_children_num,
                "Failed to find current node inside its parent children vector");
            
            lp_vector_remove_i(parent->children,curr_node_parent_i);

            if(lp_vector_empty(parent->children))
                lp_vector_push_back(release_stack,&parent);
        }

        __lpg_node_release_internals(curr_node);
        lp_slab_free(slab,curr_node);
    }

    lp_htable_release(release_black_list);
    lp_vector_release(release_stack);
}


inline bool __lpg_graph_is_native_node(const lpg_graph_t *graph, const lpg_node_t *node)
{
    const lp_slab_t *slab = __lpg_graph_slab(graph);
    const lpg_node_t *graph_slab_base = slab->__buffer;
    const lpg_node_t *graph_slab_end = graph_slab_base+slab->__total_entries*sizeof(lpg_node_t);

    return node >= graph_slab_base && node < graph_slab_end;
}


size_t __lpg_graph_nodes_hsh(const lpg_node_t **node)
{
    return lp_uni_hash((size_t)(*node));
}


bool __lpg_graph_nodes_eq(const lpg_node_t **a, const lpg_node_t **b)
{
    return *a == *b;
}