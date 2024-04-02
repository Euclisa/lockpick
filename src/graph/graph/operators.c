#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


static inline bool __lpg_node_is_child_of(const lpg_node_t *node, const lpg_node_t *parent)
{
    for(size_t child_i = 0; child_i < lpg_node_get_children_num(parent); ++child_i)
        if(lp_vector_at_type(parent->children,child_i,lpg_node_t*) == node)
            return true;
    return false;
}


void __lpg_node_record_child(lpg_node_t *parent, lpg_node_t *child)
{
    affirmf(parent,"Null parent node provided. "
                    "All parent nodes must be initialized before being connected to child nodes. "
                    "Please verify all parent node pointers are populated prior to this operation.");
    affirmf_debug(!__lpg_node_is_child_of(child,parent),"Specified node is already a child of this parent node");

    lp_vector_push_back(parent->children,&child);
}


void __lpg_node_init(lpg_node_t *node)
{
    size_t parents_num = lpg_node_get_parents_num(node);
    lpg_node_t **parents_ptr;
    if(parents_num > 0)
        parents_ptr = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*parents_num);
    else
        parents_ptr = NULL;

    __lpg_node_set_parents(node,parents_ptr);

    node->children = lp_vector_create(0,sizeof(lpg_node_t*));
}


lpg_node_t *__lpg_node_alloc(lp_slab_t *slab)
{
    lpg_node_t *node;
    if(slab)
    {
        affirmf_debug(slab->__entry_size == sizeof(lpg_node_t),
            "Got slab with invalid entry size");

        node = (lpg_node_t*)lp_slab_alloc(slab);
        affirmf(node,"Failed to allocate space for node from specified slab");
    }
    else
    {
        node = (lpg_node_t*)malloc(sizeof(lpg_node_t));
        affirmf(node,"Failed to allocate space for node from memory");
    }

    return node;
}


lpg_node_t *lpg_node_and(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"left-side node operand");
    affirm_nullptr(b,"right-side node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);

    node->type = LPG_NODE_TYPE_AND;
    __lpg_node_init(node);
    
    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);
    lpg_node_parents(node)[1] = b;
    __lpg_node_record_child(b,node);

    return node;
}

lpg_node_t *lpg_node_or(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"left-side node operand");
    affirm_nullptr(b,"right-side node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);

    node->type = LPG_NODE_TYPE_OR;
    __lpg_node_init(node);
    
    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);
    lpg_node_parents(node)[1] = b;
    __lpg_node_record_child(b,node);

    return node;
}

lpg_node_t *lpg_node_not(lpg_graph_t *graph, lpg_node_t *a)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);

    node->type = LPG_NODE_TYPE_NOT;
    __lpg_node_init(node);
    
    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);

    return node;
}

lpg_node_t *lpg_node_xor(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"left-side node operand");
    affirm_nullptr(b,"right-side node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);

    node->type = LPG_NODE_TYPE_XOR;
    __lpg_node_init(node);

    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);
    lpg_node_parents(node)[1] = b;
    __lpg_node_record_child(b,node);

    return node;
}

lpg_node_t *lpg_node_const(lpg_graph_t *graph, bool value)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);

    node->type = LPG_NODE_TYPE_CONST;
    __lpg_node_init(node);
    __lpg_node_set_value(node,value);

    return node;
}
