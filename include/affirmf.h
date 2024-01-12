#ifndef _LOCKPICK_INCLUDE_AFFIRMF_H
#define _LOCKPICK_INCLUDE_AFFIRMF_H

#include <stdbool.h>
#include <stdarg.h>

#define __LP_MAX_AFFIRMF_MSG_SIZE 256


void __affirmf(bool cond, const char *format, ...);

#define affirmf(cond,format,...) __affirmf(cond,format,#cond,__FILE__,__LINE__,__func__,##__VA_ARGS__)


#endif // _LOCKPICK_INCLUDE_AFFIRMF_H