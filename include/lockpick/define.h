#ifndef _LOCKPICK_DEFINE_H
#define _LOCKPICK_DEFINE_H

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>

#define __aligned(N) __attribute__((aligned(N)))
#define __no_return __attribute__((noreturn))

#define lp_assert(cond,msg)                     \
    assert((cond) && __func__ && msg)

#define return_set_errno(ret,errno_code)  ({    \
    errno = errno_code;                         \
    return ret;                                 \
})

#define __likely(x) __builtin_expect(!!(x), 1)
#define __unlikely(x) __builtin_expect(!!(x), 0)

#define __POSIX_MEMALIGN_MIN_ALIGNMENT (sizeof(void*))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define LP_NPOS ((size_t)-1)

#define LP_BITS_IN_BYTE 8
#define LP_BITS_PER_HEX 4

#define lp_same_type(t1,t2) __builtin_types_compatible_p(typeof(t1),typeof(t2))

#endif // _LOCKPICK_DEFINE_H