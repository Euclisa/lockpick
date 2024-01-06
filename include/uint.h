#ifndef _LOCKPICK_INCLUDE_UINT_H
#define _LOCKPICK_INCLUDE_UINT_H

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * In this implementation we use uint16_t underlying data type in order to avoid overflows during multiplication.
*/


extern const __uint128_t __LP_UINT_BASE;
extern const uint64_t __LP_UINT_MAX_WORD;
extern const uint64_t __LP_UINT_HEXES_PER_WORD;

#define __LP_UINT_BITS_PER_WORD (sizeof(uint64_t)*8)
#define __LP_UINT_BITS_PER_HEX 4

#define __LP_UINT_IS_POW_2(N) (((N) & ((N)-1)) == 0)

#define __LP_UINT_VALIDATE_WIDTH(N) ((((N) >= __LP_UINT_BITS_PER_WORD) && __LP_UINT_IS_POW_2(N)) ? (int64_t)(N/__LP_UINT_BITS_PER_WORD) : (int64_t)-1)

#define __LP_UINT_MAX_HEX_STR_REPRESENTATION(N) (__LP_UINT_VALIDATE_WIDTH(N) >= 0 ? (int64_t)(N/__LP_UINT_BITS_PER_HEX) : (int64_t)-1)


#define uint(N)                                                                                 \
struct                                                                                          \
{                                                                                               \
    uint64_t __buffer[__LP_UINT_VALIDATE_WIDTH(N)];                                             \
}

#define __array_size(arr) (sizeof(arr) / sizeof(arr[0]))

bool __lp_uint_from_hex(const char *hex_str, uint64_t *value, size_t value_size);
#define lp_uint_from_hex(value,hex_str) __lp_uint_from_hex(hex_str, (value).__buffer, __array_size((value).__buffer))

char *__lp_uint_to_hex(uint64_t *value, size_t value_size);
#define lp_uint_to_hex(value) __lp_uint_to_hex((value).__buffer, __array_size((value).__buffer))

bool __lp_uint_add(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size, uint64_t *result, size_t result_size);
#define lp_uint_add(a,b,result) __lp_uint_add((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer), (result).__buffer, __array_size((result).__buffer))

bool __lp_uint_sub(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size, uint64_t *result, size_t result_size);
#define lp_uint_sub(a,b,result) __lp_uint_sub((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer), (result).__buffer, __array_size((result).__buffer))

bool __lp_uint_mul(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size, uint64_t *result, size_t result_size);
#define lp_uint_mul(a,b,result) __lp_uint_mul((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer), (result).__buffer, __array_size((result).__buffer))

bool __lp_uint_eq(uint64_t *a, size_t a_size, uint64_t *b, size_t b_size);
#define lp_uint_eq(a,b) __lp_uint_eq((a).__buffer, __array_size((a).__buffer), (b).__buffer, __array_size((b).__buffer))


#endif  // _LOCKPICK_INCLUDE_UINT_H
