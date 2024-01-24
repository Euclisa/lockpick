#ifndef _LOCKPICK_DEFINE_H
#define _LOCKPICK_DEFINE_H

#include <assert.h>
#include <errno.h>
#include <stddef.h>

#define __aligned(N) __attribute__((aligned(N)))
#define __no_return __attribute__((noreturn))

#define lp_assert(cond,msg)                     \
    assert((cond) && __func__ && msg)

#define return_set_errno(ret,errno_code)  ({    \
    errno = errno_code;                         \
    return ret;                                 \
})

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define __POSIX_MEMALIGN_MIN_ALIGNMENT (sizeof(void*))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif // _LOCKPICK_DEFINE_H