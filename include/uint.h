#ifndef _LOCKPICK_INCLUDE_UINT_H
#define _LOCKPICK_INCLUDE_UINT_H

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * In this implementation we use uint16_t underlying data type in order to avoid overflows during multiplication.
*/


#define __UINT_BITS_PER_WORD (sizeof(uint16_t)*8)
#define __UINT_BITS_PER_HEX 4
#define __UINT_HEXES_PER_WORD (__UINT_BITS_PER_WORD/__UINT_BITS_PER_HEX)


#define __is_pow_2(N) (((N) & ((N)-1)) == 0)

#define __UINT_VALIDATE_WIDTH(N) ((((N) % __UINT_BITS_PER_WORD == 0) && __is_pow_2(N)) ? (int64_t)(N/__UINT_BITS_PER_WORD) : (int64_t)-1)

#define __UINT_MAX_HEX_STR_REPRESENTATION(N) (__UINT_VALIDATE_WIDTH(N) >= 0 ? (int64_t)(N/__UINT_BITS_PER_HEX) : (int64_t)-1)


#define uint(N)                                                                                 \
struct                                                                                          \
{                                                                                               \
    uint16_t __buffer[__UINT_VALIDATE_WIDTH(N)];                                                \
}

#define __array_size(arr) (sizeof(arr) / sizeof(arr[0]))

bool __uint_from_hex(const char *hex_str, uint16_t *value, size_t value_size);
#define uint_from_hex(value,hex_str) __uint_from_hex(hex_str, (value)->__buffer, __array_size((value)->__buffer))

char *__uint_to_hex(uint16_t *value, size_t value_size);
#define uint_to_hex(value) __uint_to_hex((value)->__buffer, __array_size((value)->__buffer))

bool __uint_add(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size, uint16_t *result, size_t result_size);
#define uint_add(a,b,result) __uint_add((a)->__buffer, __array_size((a)->__buffer), (b)->__buffer, __array_size((b)->__buffer), (result)->__buffer, __array_size((result)->__buffer))


#endif  // _LOCKPICK_INCLUDE_UINT_H
