#ifndef _LOCKPICK_STRING_H
#define _LOCKPICK_STRING_H

#define __LP_STRING_RED_STYLE_MAGIC "\033[1;31;1m"
#define __LP_STRING_GREEN_STYLE_MAGIC "\033[1;32;1m"
#define __LP_STRING_YELLOW_STYLE_MAGIC "\033[1;33;1m"
#define __LP_STRING_BLUE_STYLE_MAGIC "\033[1;34;1m"
#define __LP_STRING_RESET_STYLE_MAGIC "\033[0m"

#define lp_string_red(str)          \
    __LP_STRING_RED_STYLE_MAGIC str __LP_STRING_RESET_STYLE_MAGIC

#define lp_string_green(str)        \
    __LP_STRING_GREEN_STYLE_MAGIC str __LP_STRING_RESET_STYLE_MAGIC

#define lp_string_yellow(str)       \
    __LP_STRING_YELLOW_STYLE_MAGIC str __LP_STRING_RESET_STYLE_MAGIC

#define lp_string_blue(str)         \
    __LP_STRING_BLUE_STYLE_MAGIC str __LP_STRING_RESET_STYLE_MAGIC

#endif // _LOCKPICK_STRING_H