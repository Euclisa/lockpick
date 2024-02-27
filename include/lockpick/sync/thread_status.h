#ifndef _LOCKPICK_SYNC_THREAD_STATUS_H
#define _LOCKPICK_SYNC_THREAD_STATUS_H

#include <lockpick/sync/err.h>
#include <lockpick/list.h>


typedef struct lp_thread_status
{
    lp_list_t __list;
    __lp_err_frame_t *__frames;
} lp_thread_status_t;


lp_thread_status_t *lp_thread_status_create();

uint32_t lp_thread_join(pthread_t thread, lp_thread_status_t **status);

#endif // _LOCKPICK_SYNC_THREAD_STATUS_H