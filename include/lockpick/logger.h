#ifndef _LOCKPICK_LOGGER_H
#define _LOCKPICK_LOGGER_H

#include <stdio.h>
#include <time.h>


typedef enum lp_logger_levels
{
    LP_LOGGER_LEVEL_DEBUG,
    LP_LOGGER_LEVEL_INFO,
    LP_LOGGER_LEVEL_WARNING,
    LP_LOGGER_LEVEL_ERROR,
    LP_LOGGER_LEVEL_CRITICAL,
    LP_LOGGER_LEVEL_OFF
} lp_logger_levels_t;


/**
 * %L - log level string
 * %s - user message
 * %x - current time representation
 * %X - current date representation
 * %% - '%' symbol
*/
typedef struct lp_logger
{
    lp_logger_levels_t log_level;
    const char *format;
    time_t last_log_ts;
    size_t __format_len;
    size_t __static_log_len;
} lp_logger_t;


lp_logger_t *lp_logger_create(const char *format);

void lp_logger_set_log_level(lp_logger_t *log, lp_logger_levels_t level);

void lp_logger_debug(lp_logger_t *log, const char *format_msg, ...);
void lp_logger_info(lp_logger_t *log, const char *format_msg, ...);
void lp_logger_warning(lp_logger_t *log, const char *format_msg, ...);
void lp_logger_error(lp_logger_t *log, const char *format_msg, ...);
void lp_logger_critical(lp_logger_t *log, const char *format_msg, ...);

#endif // _LOCKPICK_LOGGER_H