#ifndef _LOCKPICK_RB_TREE_H
#define _LOCKPICK_RB_TREE_H

#include <stdint.h>
#include <lockpick/define.h>


#define lp_is_pow_2(x) (__builtin_popcountll((unsigned long long)x) == 1)

#define lp_exact_log2(x) (__builtin_ffsll((unsigned long long)x)-1)

#define lp_floor_log2(x)        \
        (LP_BITS_IN_BYTE * sizeof(unsigned long long) - __builtin_clzll((unsigned long long)x) - 1)

#define lp_ceil_log2(x)         \
        (lp_floor_log2(x) + !lp_is_pow_2(x))


uint64_t lp_ceil_div_u(uint64_t a, uint64_t b);

#endif // _LOCKPICK_RB_TREE_H