#include <lockpick/graph/graph.h>
#include <lockpick/container_of.h>
#include <lockpick/affirmf.h>
#include <lockpick/rb_tree.h>


typedef struct __lpg_node_set
{
    lp_rb_node_t __rb_node;
    lpg_node_t *node;
} __lpg_node_set_t;


lp_rb_node_t *__lpg_node_set_rb_insert(lp_rb_node_t *root, __lpg_node_set_t *entry)
{
    if(root)
    {
        __lpg_node_set_t *current = container_of(root,__lpg_node_set_t,__rb_node);
        while(1)
        {
            if(current->node == entry->node)
                return root;

            lp_rb_node_t **next_child;
            next_child = current->node < entry->node ? &current->__rb_node.right : &current->__rb_node.left;
            if(!*next_child)
            {
                *next_child = &entry->__rb_node;
                lp_rb_set_parent(&entry->__rb_node,&current->__rb_node);
                break;
            }
            current = container_of(*next_child,__lpg_node_set_t,__rb_node);
        }
    }
    else
        lp_rb_set_parent(&entry->__rb_node,NULL);
    entry->__rb_node.left = NULL;
    entry->__rb_node.right = NULL;

    return lp_rb_insert_rebalance(root,&entry->__rb_node);
}

__lpg_node_set_t *__lpg_node_set_rb_find(lp_rb_node_t *root, lpg_node_t *node)
{
    if(root)
    {
        __lpg_node_set_t *current = container_of(root,__lpg_node_set_t,__rb_node);
        while(current)
        {
            if(current->node == node)
                return current;

            lp_rb_node_t *next_child = current->node < node ? current->__rb_node.right : current->__rb_node.left;
            current = container_of(next_child,__lpg_node_set_t,__rb_node);
        }
    }

    return NULL;
}


typedef struct __lpg_dangling_slab_callback_args
{
    lp_rb_node_t *node_set_root;
    size_t counter;
} __lpg_dangling_slab_callback_args_t;


static inline void __lpg_graph_count_dangling_slab_callback(void *entry_ptr, void *args)
{
    lpg_node_t *node = (lpg_node_t*)entry_ptr;
    __lpg_dangling_slab_callback_args_t *args_struct = (__lpg_dangling_slab_callback_args_t*)args;

    if(!node->children)
    {
        if(!__lpg_node_set_rb_find(args_struct->node_set_root,node))
            ++args_struct->counter;
    }
}

inline size_t lpg_graph_count_dangling_nodes(lpg_graph_t *graph)
{
    affirmf(graph,"Expected valid graph pointer but null was given");

    lp_rb_node_t *node_set_root = NULL;
    size_t total_entries = graph->inputs_size+graph->outputs_size;
    lp_slab_t *entries_slab = lp_slab_create(total_entries,sizeof(__lpg_node_set_t));
    for(size_t in_node_i = 0; in_node_i < graph->inputs_size; ++in_node_i)
    {
        __lpg_node_set_t *entry = lp_slab_alloc(entries_slab);
        entry->node = graph->inputs[in_node_i];
        node_set_root = __lpg_node_set_rb_insert(node_set_root,entry);
    }
    for(size_t out_node_i = 0; out_node_i < graph->outputs_size; ++out_node_i)
    {
        __lpg_node_set_t *entry = lp_slab_alloc(entries_slab);
        entry->node = graph->outputs[out_node_i];
        node_set_root = __lpg_node_set_rb_insert(node_set_root,entry);
    }

    __lpg_dangling_slab_callback_args_t args;
    args.counter = 0;
    args.node_set_root = node_set_root;

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lp_slab_exec(slab,__lpg_graph_count_dangling_slab_callback,&args);

    lp_slab_release(entries_slab);

    return args.counter;
}