#ifndef _LOCKPICK_RB_TREE_H
#define _LOCKPICK_RB_TREE_H

#include <stdint.h>
#include <lockpick/define.h>

// TODO: Rewrite with "__builtin_popcountll(x) == 1"
#define lp_is_pow_2(x) (((x) & ((x)-1)) == 0)

#define lp_exact_log2(x) (__builtin_ffsll(x)-1)

#define lp_floor_log2(x)   \
        (LP_BITS_IN_BYTE * sizeof(unsigned long long) - __builtin_clzll((unsigned long long)x) - 1)


uint64_t lp_ceil_div_u(uint64_t a, uint64_t b);

#endif // _LOCKPICK_RB_TREE_H