#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>


void __lpg_graph_compute_node_cb(lpg_graph_t *graph, lpg_node_t *node, void *args)
{
    lpg_node_t **parents = lpg_node_parents(node);
    bool node_value;
    switch(node->type)
    {
        case LPG_NODE_TYPE_AND:
        {
            bool a_val = lpg_node_value(parents[0]);
            bool b_val = lpg_node_value(parents[1]);
            node_value = a_val && b_val;
            break;
        }
        case LPG_NODE_TYPE_OR:
        {
            bool a_val = lpg_node_value(parents[0]);
            bool b_val = lpg_node_value(parents[1]);
            node_value = a_val || b_val;
            break;
        }
        case LPG_NODE_TYPE_NOT:
        {
            bool a_val = lpg_node_value(parents[0]);
            node_value = !a_val;
            break;
        }
        case LPG_NODE_TYPE_XOR:
        {
            bool a_val = lpg_node_value(parents[0]);
            bool b_val = lpg_node_value(parents[1]);
            node_value = a_val ^ b_val;
            break;
        }
        case LPG_NODE_TYPE_CONST:
        {
            node_value = lpg_node_value(node);
            break;
        }
        default:
            errorf("Invalid operation type: %d",node->type);
    }

    __lpg_node_set_value(node,node_value);
}


inline void lpg_graph_compute(lpg_graph_t *graph)
{
    affirm_nullptr(graph,"graph");
    affirmf(graph,"Expected valid graph pointer but null was given");

    lpg_graph_traverse(graph,NULL,NULL,__lpg_graph_compute_node_cb,NULL);
}