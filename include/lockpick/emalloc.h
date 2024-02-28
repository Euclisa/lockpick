#ifndef _LOCKPICK_EMALLOC_H
#define _LOCKPICK_EMALLOC_H

#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <stdlib.h>


#define emalloc(elements_num,size,err_ret)  ({              \
    void *__ptr = malloc((elements_num)*(size));            \
    return_set_errno_on(!__ptr,err_ret,ENOMEM);             \
    __ptr;                                                  \
})


#define ecalloc(elements_num,size,err_ret)  ({              \
    void *__ptr = calloc((elements_num),(size));            \
    return_set_errno_on(!__ptr,err_ret,ENOMEM);             \
    __ptr;                                                  \
})


#define erealloc(ptr,elements_num,size,err_ret)  ({         \
    void *__ptr = realloc(ptr,(elements_num)*(size));       \
    return_set_errno_on(!__ptr,err_ret,ENOMEM);             \
    __ptr;                                                  \
})

#endif // _LOCKPICK_EMALLOC_H