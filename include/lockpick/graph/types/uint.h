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

lpg_uint_t *lpg_uint_allocate(lpg_graph_t *graph, size_t width);
lpg_uint_t *lpg_uint_allocate_as_view(lpg_graph_t *graph, lpg_node_t **nodes, size_t width);

void lpg_uint_update_from_nodes(lpg_uint_t *value, lpg_node_t **nodes);
void lpg_uint_update_fill_with_single(lpg_uint_t *value, lpg_node_t *node);
void lpg_uint_update_empty(lpg_uint_t *value);
void lpg_uint_update_from_hex_str(lpg_uint_t *value, const char *hex_str);

void lpg_uint_release(lpg_uint_t *_uint);

void lpg_uint_assign_from_hex_str(const char *hex_str, lpg_uint_t *value);

void __lpg_uint_update_from_uint(lpg_uint_t *value, const __lp_uint_word_t *uint_value, size_t uint_value_size);
#define lpg_uint_update_from_uint(value,uint_value) __lpg_uint_update_from_uint((value),(uint_value).__buffer,__array_size((uint_value).__buffer))

size_t lpg_uint_to_hex(const lpg_uint_t *value, char *dest, size_t n);

void lpg_uint_copy(lpg_uint_t *dest, lpg_uint_t *src);

void lpg_uint_add(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_add_ip(lpg_uint_t *dest, lpg_uint_t *other);
void lpg_uint_sub(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_mul(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);

void lpg_uint_and(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_and_ip(lpg_uint_t *dest, lpg_uint_t *other);
void lpg_uint_or(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_or_ip(lpg_uint_t *dest, lpg_uint_t *other);
void lpg_uint_xor(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result);
void lpg_uint_xor_ip(lpg_uint_t *dest, lpg_uint_t *other);

void lpg_uint_lshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result);
void lpg_uint_rshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result);

#endif // _LOCKPICK_GRAPH_TYPES_UINT_H