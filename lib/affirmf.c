#include <lockpick/affirmf.h>
#include <stdio.h>
#include <stdlib.h>


static inline void __no_return __verrorf(const char *format, const char *file_str, uint64_t line_num, const char *func_str, va_list args)
{
    char msg[__LP_MAX_AFFIRMF_MSG_SIZE+1] = {0};
    if(format)
        vsnprintf(msg,sizeof(msg),format,args);

    printf("\n%s:%ld: Unrecoverable error in function: '%s'. Details: '%s'. Bailing out...\n",file_str,line_num,func_str,msg);
    exit(1);
}

inline void __errorf(const char *format, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    va_list args;
    va_start(args,func_str);

    __verrorf(format,file_str,line_num,func_str,args);
}


inline void __affirmf(bool cond, const char *format, const char *cond_str, const char *file_str, uint64_t line_num, const char *func_str, ...)
{
    if(cond)
        return;

    va_list args;
    va_start(args,func_str);

    __verrorf(format,file_str,line_num,func_str,args);
}