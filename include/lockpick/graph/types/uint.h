#ifndef _LOCKPICK_GRAPH_TYPES_UINT_H
#define _LOCKPICK_GRAPH_TYPES_UINT_H

#include <lockpick/graph/graph.h>

#define __LPG_UINT_BITS_PER_HEX 4


typedef struct lpg_uint
{
    lpg_node_t **nodes;
    size_t width;
} lpg_uint_t;


lpg_uint_t *lpg_uint_create(size_t width);

void lpg_uint_release(lpg_uint_t *_uint);

bool lpg_uint_from_hex(lpg_graph_t *graph, const char *hex_str, lpg_uint_t *value);

int64_t lpg_uint_to_hex(lpg_uint_t *value, char *dest, size_t n);

#endif // _LOCKPICK_GRAPH_TYPES_UINT_H