#ifndef _LOCKPICK_RB_TREE_H
#define _LOCKPICK_RB_TREE_H

#include <stdint.h>

// TODO: Rewrite with "__builtin_popcountll(x) == 1"
#define lp_is_pow_2(x) (((x) & ((x)-1)) == 0)

#define lp_exact_log_2(x) (__builtin_ffsll(x)-1);

uint64_t lp_ceil_div_u(uint64_t a, uint64_t b);

#endif // _LOCKPICK_RB_TREE_H