#include <lockpick/logger.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <lockpick/utility.h>
#include <lockpick/math.h>
#include <lockpick/string.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <alloca.h>

#define __LP_LOGGER_SECONDS_LEN 2
#define __LP_LOGGER_MINUTES_LEN 2
#define __LP_LOGGER_HOURS_LEN   2
#define __LP_LOGGER_DAYS_LEN    2
#define __LP_LOGGER_MONTHS_LEN  2
#define __LP_LOGGER_YEARS_LEN   4


#define __LP_LOGGER_DEBUG_STR           lp_string_styled_lit("DEBUG",white,bold)
#define __LP_LOGGER_DEBUG_STR_LEN       (sizeof(__LP_LOGGER_DEBUG_STR)-1)

#define __LP_LOGGER_INFO_STR            lp_string_styled_lit("INFO",blue,bold)
#define __LP_LOGGER_INFO_STR_LEN        (sizeof(__LP_LOGGER_INFO_STR)-1)

#define __LP_LOGGER_WARNING_STR         lp_string_styled_lit("WARNING",yellow,bold)
#define __LP_LOGGER_WARNING_STR_LEN     (sizeof(__LP_LOGGER_WARNING_STR)-1)

#define __LP_LOGGER_ERROR_STR           lp_string_styled_lit("ERROR",red,bold)
#define __LP_LOGGER_ERROR_STR_LEN       (sizeof(__LP_LOGGER_ERROR_STR)-1)

#define __LP_LOGGER_CRITICAL_STR        lp_string_styled_lit("CRITICAL",magneta,bold)
#define __LP_LOGGER_CRITICAL_STR_LEN    (sizeof(__LP_LOGGER_CRITICAL_STR)-1)


void __lp_logger_seconds(const struct tm *timeptr, char *dest)
{
    int seconds = timeptr->tm_sec;
    dest[0] = lp_digit_to_char(seconds / 10);
    dest[1] = lp_digit_to_char(seconds % 10);
}

void __lp_logger_minutes(const struct tm *timeptr, char *dest)
{
    int minutes = timeptr->tm_min;
    dest[0] = lp_digit_to_char(minutes / 10);
    dest[1] = lp_digit_to_char(minutes % 10);
}

void __lp_logger_hours(const struct tm *timeptr, char *dest)
{
    int hours = timeptr->tm_hour;
    dest[0] = lp_digit_to_char(hours / 10);
    dest[1] = lp_digit_to_char(hours % 10);
}

void __lp_logger_days(const struct tm *timeptr, char *dest)
{
    int days = timeptr->tm_mday;
    dest[0] = lp_digit_to_char(days / 10);
    dest[1] = lp_digit_to_char(days % 10);
}

void __lp_logger_months(const struct tm *timeptr, char *dest)
{
    int months = timeptr->tm_mon;
    dest[0] = lp_digit_to_char(months / 10);
    dest[1] = lp_digit_to_char(months % 10);
}

void __lp_logger_years(const struct tm *timeptr, char *dest)
{
    int years = 1900+timeptr->tm_year;
    dest[0] = lp_digit_to_char(years / 1000);
    dest[1] = lp_digit_to_char((years % 1000) / 100);
    dest[2] = lp_digit_to_char((years % 100) / 10);
    dest[3] = lp_digit_to_char(years % 10);
}



size_t __lp_logger_format(lp_logger_t *log, const char *msg, size_t msg_len, const char *level_str, size_t level_str_len, char *result)
{
    struct tm *timeptr = NULL;
    if(result)
        timeptr = localtime(&log->last_log_ts);
    
    size_t user_msg_wildcards_count = 0;
    size_t level_wildcards_count = 0;

    size_t result_char_i = 0;
    for(size_t format_char_i = 0; format_char_i < log->__format_len; ++format_char_i)
    {
        if(log->format[format_char_i] == '%')
        {
            ++format_char_i;
            switch(log->format[format_char_i])
            {
                case 'L':
                {
                    ++level_wildcards_count;
                    affirmf(level_wildcards_count == 1,"Log entry format can't contain more than one log level string wildcard (%%L)");

                    if(result)
                        strcpy(result+result_char_i,level_str);
                    result_char_i += level_str_len;
                    break;
                }
                case 'u':
                {
                    ++user_msg_wildcards_count;
                    affirmf(user_msg_wildcards_count == 1,"Log entry format can't contain more than one user message wildcard (%%u)");

                    if(result)
                        strcpy(result+result_char_i,msg);
                    result_char_i += msg_len;
                    break;
                }
                case 's':
                {
                    if(result)
                        __lp_logger_seconds(timeptr,result+result_char_i);
                    result_char_i += __LP_LOGGER_SECONDS_LEN;
                    break;
                }
                case 'M':
                {
                    if(result)
                        __lp_logger_minutes(timeptr,result+result_char_i);
                    result_char_i += __LP_LOGGER_MINUTES_LEN;
                    break;
                }
                case 'H':
                {
                    if(result)
                        __lp_logger_hours(timeptr,result+result_char_i);
                    result_char_i += __LP_LOGGER_HOURS_LEN;
                    break;
                }
                case 'd':
                {
                    if(result)
                        __lp_logger_days(timeptr,result+result_char_i);
                    result_char_i += __LP_LOGGER_DAYS_LEN;
                    break;
                }
                case 'm':
                {
                    if(result)
                        __lp_logger_months(timeptr,result+result_char_i);
                    result_char_i += __LP_LOGGER_MONTHS_LEN;
                    break;
                }
                case 'y':
                {
                    if(result)
                        __lp_logger_years(timeptr,result+result_char_i);
                    result_char_i += __LP_LOGGER_YEARS_LEN;
                    break;
                }
                case '%':
                {
                    if(result)
                        result[result_char_i] = '%';
                    ++result_char_i;
                    break;
                }
                case '\0':
                    errorf("Unexpected end of format string");
                    break;
                default:
                    errorf("Unexpected wildcard '%%%c'",log->format[format_char_i+1]);
                    break;
            }
        }
        else
        {
            if(result)
                result[result_char_i] = log->format[format_char_i];
            ++result_char_i;
        }
    }

    affirmf(user_msg_wildcards_count == 1,"Log entry format string must contain exactly one user message wildcard (%%u)");
    affirmf(level_wildcards_count == 1,"Log entry format string must contain exactly one log level string wildcard (%%L)");

    if(result)
        result[result_char_i] = '\0';

    return result_char_i;
}


lp_logger_t *lp_logger_create(const char *format)
{
    affirm_nullptr(format,"log entry format string");

    size_t log_size = sizeof(lp_logger_t);
    lp_logger_t *log = (lp_logger_t*)malloc(log_size);
    affirm_bad_malloc(log,"logger",log_size);

    log->log_level = LP_LOGGER_LEVEL_DEBUG;
    log->format = format;
    log->__format_len = strlen(format);
    log->__static_log_len = __lp_logger_format(log,NULL,0,NULL,0,NULL);

    return log;
}


void lp_logger_set_log_level(lp_logger_t *log, lp_logger_levels_t level)
{
    affirm_nullptr(log,"logger");
    affirmf(level >= LP_LOGGER_LEVEL_DEBUG && level <= LP_LOGGER_LEVEL_CRITICAL,
        "Invalid level %d",(uint32_t)level);
    
    log->log_level = level;
}


void __lp_logger_int_to_str(int value, int n, char *dest)
{
    int modulo = lp_pow_u64(value,n-1);
    int curr_char_i = n-1;
    while(curr_char_i >= 0)
    {
        dest[curr_char_i] = lp_digit_to_char(value/modulo);
        value %= modulo;
        modulo /= 10;
    }
}


void __lp_logger_log(lp_logger_t *log, const char *level_str, size_t level_str_len, const char *format_msg, va_list args)
{
    log->last_log_ts = time(NULL);

    char *msg;
    size_t msg_len = vasprintf(&msg,format_msg,args);

    size_t log_entry_size = log->__static_log_len+msg_len+level_str_len+1;
    char *result = alloca(log_entry_size);

    __lp_logger_format(log,msg,msg_len,level_str,level_str_len,result);
    fprintf(stderr,"%s\n",result);
    
    free(msg);
}


void lp_logger_debug(lp_logger_t *log, const char *format_msg, ...)
{
    if(log->log_level > LP_LOGGER_LEVEL_DEBUG)
        return;

    va_list args;
    va_start(args,format_msg);

    __lp_logger_log(log,__LP_LOGGER_DEBUG_STR,__LP_LOGGER_DEBUG_STR_LEN,format_msg,args);
}


void lp_logger_info(lp_logger_t *log, const char *format_msg, ...)
{
    if(log->log_level > LP_LOGGER_LEVEL_INFO)
        return;

    va_list args;
    va_start(args,format_msg);

    __lp_logger_log(log,__LP_LOGGER_INFO_STR,__LP_LOGGER_INFO_STR_LEN,format_msg,args);
}


void lp_logger_warning(lp_logger_t *log, const char *format_msg, ...)
{
    if(log->log_level > LP_LOGGER_LEVEL_WARNING)
        return;

    va_list args;
    va_start(args,format_msg);

    __lp_logger_log(log,__LP_LOGGER_WARNING_STR,__LP_LOGGER_WARNING_STR_LEN,format_msg,args);
}


void lp_logger_error(lp_logger_t *log, const char *format_msg, ...)
{
    if(log->log_level > LP_LOGGER_LEVEL_ERROR)
        return;

    va_list args;
    va_start(args,format_msg);

    __lp_logger_log(log,__LP_LOGGER_ERROR_STR,__LP_LOGGER_ERROR_STR_LEN,format_msg,args);
}


void lp_logger_critical(lp_logger_t *log, const char *format_msg, ...)
{
    if(log->log_level > LP_LOGGER_LEVEL_CRITICAL)
        return;

    va_list args;
    va_start(args,format_msg);

    __lp_logger_log(log,__LP_LOGGER_CRITICAL_STR,__LP_LOGGER_CRITICAL_STR_LEN,format_msg,args);
}
