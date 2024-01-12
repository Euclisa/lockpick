#include "affirmf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


inline void __affirmf(bool cond, const char *format, ...)
{
    if(cond)
        return;

    va_list args;
    va_start(args, format);

    const char *cond_str = va_arg(args,const char*);
    const char *file_str = va_arg(args,const char*);
    uint64_t line_num = va_arg(args,uint64_t);
    const char *func_str = va_arg(args,const char*);

    char msg[__LP_MAX_AFFIRMF_MSG_SIZE+1] = {0};
    if(format)
        vsnprintf(msg,sizeof(msg),format,args);

    printf("\n%s:%ld: Control flow consistency check '%s' failed in function: '%s'. Details: '%s'. Bailing out...\n",file_str,line_num,cond_str,func_str,msg);
    exit(1);
}