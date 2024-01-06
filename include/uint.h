#ifndef _LOCKPICK_INCLUDE_UINT_H
#define _LOCKPICK_INCLUDE_UINT_H

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * In this implementation we use uint16_t underlying data type in order to avoid overflows during multiplication.
*/

typedef uint64_t __lp_uint_word_t;

extern const __uint128_t __LP_UINT_BASE;
extern const uint64_t __LP_UINT_MAX_WORD;
extern const uint64_t __LP_UINT_HEXES_PER_WORD;

#define __LP_UINT_BITS_PER_WORD (sizeof(__lp_uint_word_t)*8)
#define __LP_UINT_BITS_PER_HEX 4

#define __LP_UINT_IS_POW_2(N) (((N) & ((N)-1)) == 0)

#define __LP_UINT_VALIDATE_WIDTH(N) ((((N) >= __LP_UINT_BITS_PER_WORD) && __LP_UINT_IS_POW_2(N)) ? (int64_t)(N/__LP_UINT_BITS_PER_WORD) : (int64_t)-1)

#define __LP_UINT_MAX_HEX_STR_REPRESENTATION(N) (__LP_UINT_VALIDATE_WIDTH(N) >= 0 ? (int64_t)(N/__LP_UINT_BITS_PER_HEX) : (int64_t)-1)


#define uint(N)                                                                                 \
struct                                                                                          \
{                                                                                               \
    __lp_uint_word_t __buffer[__LP_UINT_VALIDATE_WIDTH(N)];                                             \
}

#define __array_size(arr) (sizeof(arr) / sizeof(arr[0]))

bool __lp_uint_from_hex(const char *hex_str, __lp_uint_word_t *value, size_t value_size);
#define lp_uint_from_hex(value,hex_str) __lp_uint_from_hex(hex_str, (value).__buffer, __array_size((value).__buffer))

char *__lp_uint_to_hex(__lp_uint_word_t *value, size_t value_size);
#define lp_uint_to_hex(value) __lp_uint_to_hex((value).__buffer, __array_size((value).__buffer))

bool __lp_uint_add(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size);
#define lp_uint_add(a,b,result) __lp_uint_add((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer), (result).__buffer, __array_size((result).__buffer))

bool __lp_uint_sub(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size);
#define lp_uint_sub(a,b,result) __lp_uint_sub((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer), (result).__buffer, __array_size((result).__buffer))

bool __lp_uint_mul(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size);
#define lp_uint_mul(a,b,result) __lp_uint_mul((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer), (result).__buffer, __array_size((result).__buffer))

bool __lp_uint_eq(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size);
#define lp_uint_eq(a,b) __lp_uint_eq((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))


typedef enum lp_uint_3way
{
    LP_UINT_EQUAL,
    LP_UINT_LESS,
    LP_UINT_GREATER
} lp_uint_3way_t;

lp_uint_3way_t __lp_uint_3way(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size);
#define lp_uint_3way(a,b) __lp_uint_3way((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))

bool __lp_uint_ls(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size);
#define lp_uint_ls(a,b) __lp_uint_ls((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))

bool __lp_uint_leq(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size);
#define lp_uint_leq(a,b) __lp_uint_leq((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))

bool __lp_uint_gt(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size);
#define lp_uint_gt(a,b) __lp_uint_gt((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))

bool __lp_uint_geq(__lp_uint_word_t *a, size_t a_size, __lp_uint_word_t *b, size_t b_size);
#define lp_uint_geq(a,b) __lp_uint_geq((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))


#endif  // _LOCKPICK_INCLUDE_UINT_H
