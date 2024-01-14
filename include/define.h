#ifndef _LOCKPICK_INCLUDE_DEFINE_H
#define _LOCKPICK_INCLUDE_DEFINE_H

#define LOCKPICK_DEBUG

#define NULL ((void*)0)

#define aligned(N) __attribute__((aligned(N)))

#define lp_assert(cond,msg) assert((cond) && __func__ && msg)

#endif // LOCKPICK_INCLUDE_STDDEF_H