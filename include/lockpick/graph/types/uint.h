#ifndef _LOCKPICK_GRAPH_TYPES_UINT_H
#define _LOCKPICK_GRAPH_TYPES_UINT_H

#include <lockpick/graph/graph.h>


typedef struct lpg_uint
{
    lpg_node_t *nodes;
    size_t width;
} lpg_uint_t;

#endif // _LOCKPICK_GRAPH_TYPES_UINT_H