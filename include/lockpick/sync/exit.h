#ifndef _LOCKPICK_SYNC_EXIT_H
#define _LOCKPICK_SYNC_EXIT_H

#include <sys/types.h>
#include <signal.h>
#include <lockpick/define.h>


extern pid_t __main_pid;

void __lp_exit_init();
void __no_return lp_exit();

#endif // _LOCKPICK_SYNC_EXIT_H