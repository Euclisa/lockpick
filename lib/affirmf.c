#include <lockpick/affirmf.h>
#include <lockpick/sync/exit.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <libunwind.h>
#include <errno.h>
#include <string.h>


pthread_mutex_t __err_lock;


static inline void __print_backtrace(uint32_t offset)
{
    unw_context_t context;
    unw_cursor_t cursor;
    unw_getcontext(&context);
    unw_init_local(&cursor,&context);
    for(uint32_t i = 0; i < offset; ++i)
        unw_step(&cursor);
    size_t frame_i = 0;
    while(unw_step(&cursor))
    {
        #define MAX_PROC_NAME_LEN 255

        char proc_name[MAX_PROC_NAME_LEN];
        unw_word_t off;
        unw_get_proc_name(&cursor,proc_name,MAX_PROC_NAME_LEN,&off);
        fprintf(stderr,"#%zd: %s\n",frame_i,proc_name);
        ++frame_i;
    }
}


static inline void __no_return __verrorf(const char *format, const char *base_msg, const char *file_str, uint64_t line_num, const char *func_str, va_list args)
{
    pthread_mutex_lock(&__err_lock);
    char msg[__LP_MAX_AFFIRMF_MSG_SIZE+1] = {0};
    if(format)
        vsnprintf(msg,sizeof(msg),format,args);

    fprintf(stderr,"\n%s:%ld: %s in function: '%s'. Details: '%s'. Bailing out...\n\n",file_str,line_num,base_msg,func_str,msg);
    fprintf(stderr,"Errno: '%s'\n\n",strerror(errno));
    fprintf(stderr,"Backtrace:\n");
    __print_backtrace(2);
    lp_exit();
}

inline void __errorf(const char *format, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    va_list args;
    va_start(args,func_str);

    const char *base_msg = "Unrecoverable error";
    __verrorf(format,base_msg,file_str,line_num,func_str,args);
}


inline void __affirmf(const char *format, const char *cond_str, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    va_list args;
    va_start(args,func_str);

    const char *base_msg = "Control flow consistency check failed";
    __verrorf(format,base_msg,file_str,line_num,func_str,args);
}


void __lp_affirmf_init()
{
    pthread_mutex_init(&__err_lock,NULL);
}