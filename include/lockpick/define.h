#ifndef _LOCKPICK_DEFINE_H
#define _LOCKPICK_DEFINE_H

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdbool.h>

#define __aligned(N) __attribute__((aligned(N)))
#define __no_return __attribute__((noreturn))
#define __packed __attribute__((packed))

#define lp_assert(cond,msg)                             \
    assert((cond) && __func__ && msg)

#define return_set_errno(ret,errno_code)  ({            \
    errno = errno_code;                                 \
    return ret;                                         \
})

#define return_set_errno_on(cond,ret,errno_code)  ({    \
    if(cond)                                            \
    {                                                   \
        errno = errno_code;                             \
        return ret;                                     \
    }                                                   \
})

#define check_fatal(f_call,code,ret)                    \
    return_set_errno_on((f_call) == (code),ret,ENOTRECOVERABLE)

#ifdef LOCKPICK_DEBUG
#define check_fatal_debug(f_call,code,ret)              \
    check_fatal(f_call,code,ret)
#else
#define check_fatal_debug(f_call,code,ret)              \
    f_call;
#endif // LOCKPICK_DEBUG

#define return_on(cond,ret)  ({                         \
    if(cond)                                            \
        return ret;                                     \
})

#define __likely(x) __builtin_expect(!!(x), 1)
#define __unlikely(x) __builtin_expect(!!(x), 0)

#define __POSIX_MEMALIGN_MIN_ALIGNMENT (sizeof(void*))

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define LP_NPOS ((size_t)-1ULL)

#define LP_BITS_PER_BYTE 8
#define LP_BITS_PER_HEX 4

#define lp_sizeof_bits(x) (sizeof(x)*LP_BITS_PER_BYTE)

#define lp_same_type(t1,t2) __builtin_types_compatible_p(typeof(t1),typeof(t2))

#endif // _LOCKPICK_DEFINE_H