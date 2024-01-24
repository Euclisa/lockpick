#ifndef _LOCKPICK_GRAPH_TYPES_UINT_H
#define _LOCKPICK_GRAPH_TYPES_UINT_H

#include <lockpick/graph/graph.h>
#include <lockpick/uint.h>

#define __LPG_UINT_BITS_PER_HEX 4


typedef struct lpg_uint
{
    lpg_graph_t *graph;
    lpg_node_t **nodes;
    size_t width;
} lpg_uint_t;


lpg_uint_t *lpg_uint_create(lpg_graph_t *graph, lpg_node_t **nodes, size_t width);

void lpg_uint_release(lpg_uint_t *_uint);

void lpg_uint_from_hex(const char *hex_str, lpg_uint_t *value);

void __lpg_uint_from_uint(lpg_uint_t *value, const __lp_uint_word_t *uint_value, size_t uint_value_size);
#define lpg_uint_from_uint(value,uint_value) __lpg_uint_from_uint((value),(uint_value).__buffer,__array_size((uint_value).__buffer))

size_t lpg_uint_to_hex(const lpg_uint_t *value, char *dest, size_t n);

void lpg_uint_add(lpg_graph_t *graph, lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);

#endif // _LOCKPICK_GRAPH_TYPES_UINT_H