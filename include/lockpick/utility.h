#ifndef _LOCKPICK_UTILITY_H
#define _LOCKPICK_UTILITY_H

#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <stdlib.h>
#include <string.h>

#define lp_swap(a,b)  ({                                \
    static_assert(lp_same_type(a,b),                    \
            "operands types mismatch in lp_swap()");    \
    typeof(a) c = (a);                                  \
    (a) = (b);                                          \
    (b) = (c);                                          \
})


#define lp_shuffle(arr,n)  ({                           \
    for(size_t i = 0; i < (n); ++i)                     \
    {                                                   \
        size_t dest_i = rand()%(n);                     \
        lp_swap((arr)[i],(arr)[dest_i]);                \
    }                                                   \
})


size_t lp_uni_hash(size_t x);

#endif // _LOCKPICK_UTILITY_H