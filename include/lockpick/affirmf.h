#ifndef _LOCKPICK_AFFIRMF_H
#define _LOCKPICK_AFFIRMF_H

#include <lockpick/define.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

#define __LP_MAX_AFFIRMF_MSG_SIZE 256

void __no_return __errorf(const char *format, const char *file_str, uint64_t line_num, const char *func_str, ...);

void __affirmf(bool cond, const char *format, const char *cond_str, const char *file_str, uint64_t line_num, const char *func_str, ...);

#define affirmf(cond,format,...) __affirmf(cond,format,#cond,__FILE__,__LINE__,__func__,##__VA_ARGS__)

#ifdef LOCKPICK_DEBUG
#define affirmf_debug(cond,format,...) affirmf(cond,format,##__VA_ARGS__)
#else
#define affirmf_debug(cond,format,...)
#endif  // LOCKPICK_DEBUG

#define errorf(format,...) __errorf(format,__FILE__,__LINE__,__func__,##__VA_ARGS__)


#define affirm_nullptr(ptr,desc)                \
        affirmf(ptr,"Expected valid pointer on %s, but null was given",desc);

#define affirm_bad_malloc(ptr,desc,size)        \
        affirmf(ptr,"Failed to allocate memory for %s of size %ld bytes",desc,size)

#endif // _LOCKPICK_AFFIRMF_H