#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


static inline bool __lpg_node_is_child_of(const lpg_node_t *node, const lpg_node_t *parent)
{
    for(size_t child_i = 0; child_i < parent->children_size; ++child_i)
        if(parent->children[child_i] == node)
            return true;
    return false;
}


void __lpg_node_record_child(lpg_node_t *parent, lpg_node_t *child)
{
    affirmf(parent,"Null parent node provided. "
                    "All parent nodes must be initialized before being connected to child nodes. "
                    "Please verify all parent node pointers are populated prior to this operation.");
    affirmf_debug(!__lpg_node_is_child_of(child,parent),"Specified node is already a child of this parent node");

    parent->children = (lpg_node_t**)realloc(parent->children,(parent->children_size+1)*sizeof(lpg_node_t*));
    affirmf(parent->children,"Failed to allocate space for a new child");
    parent->children[parent->children_size] = child;
    ++parent->children_size;
}


void __lpg_node_general_init(lpg_node_t *node, size_t parents_num)
{
    lpg_node_t **parents_ptr;
    if(parents_num > 0)
        parents_ptr = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*parents_num);
    else
        parents_ptr = NULL;

    __lpg_node_set_parents(node,parents_ptr);
    __lpg_node_set_computed(node,false);

    node->children_size = 0;
    node->children = NULL;
}


void __lpg_node_init_and(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    __lpg_node_general_init(node,2);

    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);
    lpg_node_parents(node)[1] = b;
    __lpg_node_record_child(b,node);

    __lpg_node_set_computed(node,false);

    node->type = LPG_NODE_TYPE_AND;
}

void __lpg_node_init_or(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    __lpg_node_general_init(node,2);

    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);
    lpg_node_parents(node)[1] = b;
    __lpg_node_record_child(b,node);

    __lpg_node_set_computed(node,false);

    node->type = LPG_NODE_TYPE_OR;
}

void __lpg_node_init_not(lpg_node_t *node, lpg_node_t* a)
{
    __lpg_node_general_init(node,1);

    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);

    __lpg_node_set_computed(node,false);

    node->type = LPG_NODE_TYPE_NOT;
}

void __lpg_node_init_xor(lpg_node_t *node, lpg_node_t* a, lpg_node_t* b)
{
    __lpg_node_general_init(node,2);

    lpg_node_parents(node)[0] = a;
    __lpg_node_record_child(a,node);
    lpg_node_parents(node)[1] = b;
    __lpg_node_record_child(b,node);

    __lpg_node_set_computed(node,false);

    node->type = LPG_NODE_TYPE_XOR;
}

void __lpg_node_init_const(lpg_node_t *node, bool value)
{
    __lpg_node_general_init(node,0);

    __lpg_node_set_computed(node,true);
    __lpg_node_set_value(node,value);

    node->type = LPG_NODE_TYPE_VAR;
}

void __lpg_node_init_var(lpg_node_t *node)
{
    __lpg_node_general_init(node,0);

    __lpg_node_set_computed(node,false);

    node->type = LPG_NODE_TYPE_VAR;
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
    __lpg_node_init_and(node,a,b);

    return node;
}

lpg_node_t *lpg_node_or(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"left-side node operand");
    affirm_nullptr(b,"right-side node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_or(node,a,b);

    return node;
}

lpg_node_t *lpg_node_not(lpg_graph_t *graph, lpg_node_t *a)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_not(node,a);

    return node;
}

lpg_node_t *lpg_node_xor(lpg_graph_t *graph, lpg_node_t *a, lpg_node_t *b)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(a,"left-side node operand");
    affirm_nullptr(b,"right-side node operand");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_xor(node,a,b);

    return node;
}

lpg_node_t *lpg_node_const(lpg_graph_t *graph, bool value)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_const(node,value);

    return node;
}

lpg_node_t *lpg_node_var(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");

    lp_slab_t *slab = __lpg_graph_slab(graph);
    lpg_node_t *node = __lpg_node_alloc(slab);
    __lpg_node_init_var(node);

    return node;
}