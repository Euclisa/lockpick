#include <lockpick/sync/exit.h>
#include <stdlib.h>
#include <stdbool.h>

pid_t __main_pid;
_Atomic bool __main_running = false;

#define SIGLPEXIT SIGUSR1


void __lp_main_exit_handler(int signum)
{
    exit(1);
}


void __lp_exit_init()
{
    if(__sync_bool_compare_and_swap(&__main_running,false,true))
    {
        struct sigaction act;
        act.sa_handler = __lp_main_exit_handler;
        sigaction(SIGLPEXIT,&act,NULL);
        __main_pid = getpid();
    }
}


void __no_return lp_exit()
{
    if(__sync_bool_compare_and_swap(&__main_running,true,false))
        kill(__main_pid,SIGLPEXIT);
    exit(1);
}