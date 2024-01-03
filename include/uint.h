#ifndef _LOCKPICK_INCLUDE_UINT_H
#define _LOCKPICK_INCLUDE_UINT_H

#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

/**
 * In this implementation we use uint16_t underlying data type in order to avoid overflows during multiplication.
*/

#define __is_pow_2(N) (((N) & ((N)-1)) == 0)

#define uint(N)                                                                                 \
static_assert(N % 16 == 0 && __is_pow_2(N) && "N must be a power of 2 and greate than 16");     \
struct                                                                                          \
{                                                                                               \
    uint16_t __buffer[N/16];                                                                    \
}

#define __array_size(arr) (sizeof(arr) / sizeof(arr[0]))

bool __uint_from_hex(const char *hex_str, uint16_t *value, size_t value_size);
#define uint_from_hex(value,hex_str) __uint_from_hex(hex_str, (value)->__buffer, __array_size((value)->__buffer))

char *__uint_to_hex(uint16_t *value, size_t value_size);
#define uint_to_hex(value) __uint_to_hex((value)->__buffer, __array_size((value)->__buffer))

bool __uint_add(uint16_t *a, uint32_t a_size, uint16_t *b, uint32_t b_size, uint16_t *result, uint32_t result_size);
#define uint_add(a,b,result) __uint_add((a)->__buffer, __array_size((a)->__buffer), (b)->__buffer, __array_size((b)->__buffer), (result)->__buffer, __array_size((result)->__buffer))


#endif  // _LOCKPICK_INCLUDE_UINT_H
