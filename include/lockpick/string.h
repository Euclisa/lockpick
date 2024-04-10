#ifndef _LOCKPICK_STRING_H
#define _LOCKPICK_STRING_H

#include <string.h>
#include <alloca.h>

#define __LP_STRING_RESET_STYLE_MAGIC   "\033[0m"

#define __lp_string_ccommon_code     "29"
#define __lp_string_cgray_code       "30"
#define __lp_string_cred_code        "31"
#define __lp_string_cgreen_code      "32"
#define __lp_string_cyellow_code     "33"
#define __lp_string_cblue_code       "34"
#define __lp_string_cmagneta_code    "35"
#define __lp_string_ccyan_code       "36"
#define __lp_string_cwhite_code      "37"

#define __lp_string_fbold_code       "1"
#define __lp_string_fdim_code        "2"
#define __lp_string_fitalic_code     "3"
#define __lp_string_funderline_code  "4"
#define __lp_string_fcommon_code     "5"

#define __LP_STRING_SET_MAGIC_LEN   (sizeof("\033[0;00;0m")-1)
#define __LP_STRING_RESET_MAGIC_LEN (sizeof("\033[0m")-1)


#define lp_string_styled(str,color,format)  ({                                                 \
    size_t str_len = strlen(str);                                                              \
    const char *style_escape = "\033[" __lp_string_f##format##_code ";"                        \
                                    __lp_string_c##color##_code ";"                            \
                                     __lp_string_f##format##_code "m";                         \
    size_t styled_str_len =                                                                    \
        __LP_STRING_SET_MAGIC_LEN+str_len+__LP_STRING_RESET_MAGIC_LEN;                         \
    char *styled_str =                                                                         \
        (char*)alloca(styled_str_len+1);                                                       \
    strcpy(styled_str,style_escape);                                                           \
    strcpy(styled_str+__LP_STRING_SET_MAGIC_LEN,str);                                          \
    strcpy(styled_str+__LP_STRING_SET_MAGIC_LEN+str_len,__LP_STRING_RESET_STYLE_MAGIC);        \
    styled_str;                                                                                \
})


#define lp_string_styled_lit(str,color,format) "\033[" __lp_string_f##format##_code ";" __lp_string_c##color##_code ";" __lp_string_f##format##_code "m" str __LP_STRING_RESET_STYLE_MAGIC


size_t lp_string_hash(const char *str);

#endif // _LOCKPICK_STRING_H