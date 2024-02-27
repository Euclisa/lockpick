#include <lockpick/sync/thread_status.h>
#include <lockpick/container_of.h>
#include <stdlib.h>


lp_thread_status_t *lp_thread_status_create()
{
    lp_list_t *frames = (lp_list_t*)pthread_getspecific(__lp_err);
    if(!frames)
        return NULL;
    
    lp_thread_status_t *status = (lp_thread_status_t*)malloc(sizeof(lp_thread_status_t));
    status->__frames = container_of(frames,__lp_err_frame_t,__list);

    return status;
}


uint32_t lp_thread_join(pthread_t thread, lp_thread_status_t **status)
{
    lp_thread_status_t *joined_status;
    uint32_t ret_code = pthread_join(thread,(void**)&joined_status);
    if(ret_code == 0 && joined_status)
    {
        lp_list_t *status_head = &(*status)->__list;
        lp_list_t *joined_status_head = &joined_status->__list;
        lp_list_push_back(&status_head,joined_status_head);
        *status = container_of(status_head,lp_thread_status_t,__list);
    }

    return ret_code;
}