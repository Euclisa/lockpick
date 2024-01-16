#include <lockpick/affirmf.h>
#include <stdio.h>
#include <stdlib.h>


inline void __affirmf(bool cond, const char *format, const char *cond_str, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    if(cond)
        return;

    va_list args;
    va_start(args, func_str);

    char msg[__LP_MAX_AFFIRMF_MSG_SIZE+1] = {0};
    if(format)
        vsnprintf(msg,sizeof(msg),format,args);

    printf("\n%s:%ld: Control flow consistency check '%s' failed in function: '%s'. Details: '%s'. Bailing out...\n",file_str,line_num,cond_str,func_str,msg);
    exit(1);
}