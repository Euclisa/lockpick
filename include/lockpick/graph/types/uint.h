#ifndef _LOCKPICK_GRAPH_TYPES_UINT_H
#define _LOCKPICK_GRAPH_TYPES_UINT_H

#include <lockpick/graph/graph.h>
#include <lockpick/uint.h>

#define __LPG_UINT_BITS_PER_HEX 4


typedef struct lpg_uint
{
    lpg_graph_t *graph;
    uintptr_t __nodes_own;
    size_t width;
} lpg_uint_t;


lpg_node_t **lpg_uint_nodes(const lpg_uint_t *value);

lpg_uint_t *lpg_uint_create_from_nodes(lpg_graph_t *graph, lpg_node_t **nodes, size_t width);
lpg_uint_t *lpg_uint_create_fill_with_single(lpg_graph_t *graph, lpg_node_t *node, size_t width);
lpg_uint_t *lpg_uint_create_empty(lpg_graph_t *graph, size_t width);
lpg_uint_t *lpg_uint_create_as_copy(lpg_uint_t *src);
lpg_uint_t *lpg_uint_create_from_hex_str(lpg_graph_t *graph, const char *hex_str, size_t width);
lpg_uint_t *lpg_uint_create_as_view(lpg_graph_t *graph, lpg_node_t **nodes, size_t width);

void lpg_uint_release(lpg_uint_t *_uint);

void lpg_uint_update_from_hex(const char *hex_str, lpg_uint_t *value);

void __lpg_uint_update_from_uint(lpg_uint_t *value, const __lp_uint_word_t *uint_value, size_t uint_value_size);
#define lpg_uint_update_from_uint(value,uint_value) __lpg_uint_update_from_uint((value),(uint_value).__buffer,__array_size((uint_value).__buffer))

size_t lpg_uint_to_hex(const lpg_uint_t *value, char *dest, size_t n);

void lpg_uint_copy(lpg_uint_t *dest, lpg_uint_t *src);

void lpg_uint_add(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_add_ip(lpg_uint_t *dest, lpg_uint_t *other);
void lpg_uint_sub(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_mul(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);

void lpg_uint_and(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_or(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_xor(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);

void lpg_uint_lshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result);
void lpg_uint_rshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result);

#endif // _LOCKPICK_GRAPH_TYPES_UINT_H