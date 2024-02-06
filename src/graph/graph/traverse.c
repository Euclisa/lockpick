#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/list.h>
#include <lockpick/container_of.h>
#include <lockpick/htable.h>
#include <lockpick/utility.h>


typedef struct __lp_node_stack
{
    lp_list_t __list;
    lpg_node_t *node;
} __lp_node_stack_t;


static inline bool __lpg_node_stack_push(__lp_node_stack_t **stack, lpg_node_t *node)
{
    __lp_node_stack_t *entry = (__lp_node_stack_t*)malloc(sizeof(__lp_node_stack_t));
    entry->node = node;
    lp_list_t *old_head = *stack ? &(*stack)->__list : NULL;
    affirmf(lp_list_push_front(&old_head,&entry->__list),"Failed to push to node computing stack");
    
    *stack = container_of(old_head,__lp_node_stack_t,__list);

    return true;
}

static inline void __lpg_node_stack_pop(__lp_node_stack_t **stack)
{
    lp_list_t *old_head = &(*stack)->__list;
    __lp_node_stack_t *top = (*stack);
    affirmf(lp_list_remove(&old_head,&(*stack)->__list),"Failed to remove top from node computing stack");

    *stack = container_of(old_head,__lp_node_stack_t,__list);
    free(top);
}


size_t __lpg_graph_nodes_ht_hsh(const void *x)
{
    const lpg_node_t **node = (const lpg_node_t**)x;
    size_t h = lp_uni_hash((size_t)(*node));

    return h;
}

bool __lpg_graph_nodes_ht_eq(const void *a, const void *b)
{
    const lpg_node_t **node_a = (const lpg_node_t**)a;
    const lpg_node_t **node_b = (const lpg_node_t**)b;

    return *node_a == *node_b;
}


/**
 * __lpg_graph_traverse_node - Internal graph DFS traversal 
 * @graph: Graph object  
 * @node: Starting node for traversal   
 * @enter_cb: Callback on first reaching node
 * @enter_args: Optional enter callback arguments
 * @leave_cb: Callback on second reach of node
 * @leave_args: Optional leave callback arguments
 *
 * Performs a Depth First Search (DFS) traversal of @graph starting  
 * at @node, invoking the given @enter_cb and @leave_cb along branches.
 * 
 * Traversal ends at constant nodes or graph input nodes. Input nodes
 * may have parents but they will not be visited via DFS.
 *
 * On first reaching a node, @enter_cb is invoked, then node is scheduled
 * for traverse for the second time after all parent subtrees are traversed.
 * On second reach, @leave_cb is invoked. Callbacks pass graph, node, args.
 *
 * WARNING: This is internal API only, intended for implementing  
 * higher level graph algorithms. Users should not call directly.
 *
 * Return: None 
*/
void __lpg_graph_traverse_node(lpg_graph_t *graph, lpg_node_t *node, __lpg_traverse_cb_t enter_cb, void *enter_cb_args, __lpg_traverse_cb_t leave_cb, void *leave_cb_args)
{
    lp_htable_t *visited_ht = lp_htable_create(
            sizeof(lpg_node_t*),
            1,
            __lpg_graph_nodes_ht_hsh,
            __lpg_graph_nodes_ht_eq);
    // Input nodes buffer size is a lower bound for final visited htable size
    lp_htable_rehash(visited_ht,graph->inputs_size);

    lp_htable_t *inputs_ht = lp_htable_create(
            sizeof(lpg_node_t*),
            1,
            __lpg_graph_nodes_ht_hsh,
            __lpg_graph_nodes_ht_eq);
    lp_htable_rehash(inputs_ht,graph->inputs_size);
    
    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
        lp_htable_insert(inputs_ht,graph->inputs+in_node_i);

    __lp_node_stack_t *stack = NULL;
    __lpg_node_stack_push(&stack,node);
    
    while(stack)
    {
        lpg_node_t *curr_node = stack->node;
        __lpg_node_stack_pop(&stack);

        if(!lp_htable_find(visited_ht,&curr_node))
        {
            lp_htable_insert(visited_ht,&curr_node);
            __lpg_node_stack_push(&stack,curr_node);

            if(enter_cb)
                enter_cb(graph,curr_node,enter_cb_args);

            if(!lp_htable_find(inputs_ht,&curr_node))
            {
                uint16_t curr_node_parents_num = lpg_node_get_parents_num(curr_node);
                lpg_node_t **curr_node_parents = lpg_node_parents(curr_node);
                for(uint16_t parent_i = 0; parent_i < curr_node_parents_num; ++parent_i)
                {
                    lpg_node_t *parent = curr_node_parents[parent_i];
                    bool is_parent_visited = lp_htable_find(visited_ht,&parent);
                    if(!is_parent_visited)
                        __lpg_node_stack_push(&stack,parent);
                }
            }
        }
        else
        {
            if(leave_cb)
                leave_cb(graph,curr_node,leave_cb_args);
        }
    }

    lp_htable_release(visited_ht);
    lp_htable_release(inputs_ht);
}