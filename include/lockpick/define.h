#ifndef _LOCKPICK_DEFINE_H
#define _LOCKPICK_DEFINE_H

#include <assert.h>
#include <errno.h>

#define LOCKPICK_DEBUG

#define NULL ((void*)0)

#define aligned(N) __attribute__((aligned(N)))

#define lp_assert(cond,msg)                     \
    assert((cond) && __func__ && msg)

#define return_set_errno(ret,errno_code)  ({    \
    errno = errno_code;                         \
    return ret;                                 \
})

#endif // _LOCKPICK_DEFINE_H