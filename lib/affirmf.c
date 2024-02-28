#include <lockpick/affirmf.h>
#include <lockpick/sync/exit.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


pthread_mutex_t __err_lock;


static inline void __no_return __verrorf(const char *format, const char *base_msg, const char *file_str, uint64_t line_num, const char *func_str, va_list args)
{
    pthread_mutex_lock(&__err_lock);
    char msg[__LP_MAX_AFFIRMF_MSG_SIZE+1] = {0};
    if(format)
        vsnprintf(msg,sizeof(msg),format,args);

    printf("\n%s:%ld: %s in function: '%s'. Details: '%s'. Bailing out...\n",file_str,line_num,base_msg,func_str,msg);
    lp_exit();
}

inline void __errorf(const char *format, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    va_list args;
    va_start(args,func_str);

    const char *base_msg = "Unrecoverable error";
    __verrorf(format,base_msg,file_str,line_num,func_str,args);
}


inline void __affirmf(bool cond, const char *format, const char *cond_str, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    if(cond)
        return;

    va_list args;
    va_start(args,func_str);

    const char *base_msg = "Control flow consistency check failed";
    __verrorf(format,base_msg,file_str,line_num,func_str,args);
}


void __lp_affirmf_init()
{
    pthread_mutex_init(&__err_lock,NULL);
}