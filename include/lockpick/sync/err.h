#ifndef _LOCKPICK_SYNC_ERR_H
#define _LOCKPICK_SYNC_ERR_H

#include <lockpick/list.h>
#include <pthread.h>
#include <stdint.h>


extern pthread_key_t __lp_err;

typedef struct __lp_err_frame
{
    lp_list_t __list;
    uint32_t __line;
    char *__file;
    char *__func;
    char *__msg;
} __lp_err_frame_t;


void __lp_err_reportf(uint32_t line, const char *file_str, const char *func_str, const char *msgf, ...);

#define lp_err_reportf(msgf,...)        \
    __lp_err_reportf(__LINE__,__FILE__,__func__,msgf,##__VA_ARGS__)

void lp_err_resolve();

void lp_err_terminate();


#endif // _LOCKPICK_SYNC_ERR_H