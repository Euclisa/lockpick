#ifndef _LOCKPICK_LOCKPICK_H
#define _LOCKPICK_LOCKPICK_H

#include <lockpick/sync/exit.h>
#include <lockpick/affinity.h>
#include <lockpick/logger.h>


extern bool lp_initialized;

extern lp_logger_t *lp_log;

void lp_init();

#endif // _LOCKPICK_LOCKPICK_H