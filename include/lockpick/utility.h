#ifndef _LOCKPICK_UTILITY_H
#define _LOCKPICK_UTILITY_H

#include <lockpick/define.h>

#define lp_swap(a,b)  ({                                \
    static_assert(lp_same_type(a,b),                    \
            "operands types mismatch in lp_swap()");    \
    typeof(a) c = (a);                                  \
    (a) = (b);                                          \
    (b) = (c);                                          \
})

size_t lp_uni_hash(size_t x);

#endif // _LOCKPICK_UTILITY_H