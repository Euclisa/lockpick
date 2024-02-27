#ifndef _LOCKPICK_RB_TREE_H
#define _LOCKPICK_RB_TREE_H

#include <stdint.h>
#include <lockpick/define.h>


#define lp_is_pow_2(x) (__builtin_popcountll((unsigned long long)(x)) == 1)

#define lp_exact_log2(x) (__builtin_ffsll((unsigned long long)(x))-1)

#define lp_floor_log2(x)        \
        (LP_BITS_PER_BYTE * sizeof(unsigned long long) - __builtin_clzll((unsigned long long)(x)) - 1)

#define lp_ceil_log2(x)         \
        (lp_floor_log2(x) + !lp_is_pow_2(x))

#define lp_div_pow_2(a,b)       \
        ((a) >> lp_floor_log2(b))

#define lp_mod_pow_2(a,b)       \
        ((a) & ((b)-1))

uint64_t lp_ceil_div_u64(uint64_t a, uint64_t b);

uint64_t lp_pow_u64(uint64_t base, uint64_t exp);

#endif // _LOCKPICK_RB_TREE_H