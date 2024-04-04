#include <lockpick/graph/ocl_graph.h>


uint16_t lpg_node_packed_get_parents_num(const lpg_node_packed_t *node)
{
    static const uint16_t node_type_parents_num[] = {2, 2, 1, 2, 0, 0};

    return node_type_parents_num[node->type];
}