#include <lockpick/sync/err.h>
#include <lockpick/container_of.h>
#include <lockpick/emalloc.h>
#include <lockpick/string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


pthread_key_t __lp_err;


void __lp_err_frame_destructor(void *_err)
{
    lp_list_t *err_head = (lp_list_t*)_err;
    lp_list_t *current = err_head->next;
    while(current != err_head)
    {
        __lp_err_frame_t *to_free = container_of(current,__lp_err_frame_t,__list);
        current = current->next;

        free(to_free->__file);
        free(to_free->__func);
        free(to_free->__msg);
        free(to_free);
    }

    __lp_err_frame_t *err_head_frame = container_of(err_head,__lp_err_frame_t,__list);
    free(err_head_frame->__file);
    free(err_head_frame->__func);
    free(err_head_frame->__msg);
    free(err_head_frame);
}

bool lp_err_init()
{
    if(pthread_key_create(&__lp_err,NULL) != 0)
        return_set_errno(false,ENOTRECOVERABLE);
    return true;
}


void __lp_err_frame_push(uint32_t line, const char *file_str, const char *func_str, char *msg_str)
{
    __lp_err_frame_t *frame = (__lp_err_frame_t*)malloc(sizeof(__lp_err_frame_t));

    frame->__file = strdup(file_str);
    frame->__func = strdup(func_str);
    frame->__msg = msg_str;
    frame->__line = line;

    lp_list_t *err_frames = pthread_getspecific(__lp_err);
    lp_list_push_front(&err_frames,&frame->__list);
    pthread_setspecific(__lp_err,err_frames);
}


void __lp_err_reportf(uint32_t line, const char *file_str, const char *func_str, const char *msgf, ...)
{
    va_list args;
    va_start(args,msgf);

    char *msg_str;
    vasprintf(&msg_str,msgf,args);

    __lp_err_frame_push(line,file_str,func_str,msg_str);
}


void lp_err_resolve()
{
    void *err_frames = pthread_getspecific(__lp_err);
    __lp_err_frame_destructor(err_frames);
    pthread_setspecific(__lp_err,NULL);
}


void lp_err_terminate()
{
    const char *error_header = lp_string_red("Unrecoverable error occured");
    fprintf(stderr,"%s\n",error_header);

    lp_list_t *err_frames_head = pthread_getspecific(__lp_err);
    if(err_frames_head)
    {
        err_frames_head = err_frames_head->prev;
        size_t frame_i = 0;
        lp_list_foreach_rev(err_frames_head,frame,__lp_err_frame_t,__list)
            fprintf(stderr,"\t#%ld %s:%d in %s: '%s'\n",frame_i,frame->__file,frame->__line,frame->__func,frame->__msg);
    }
    
    exit(1);
}