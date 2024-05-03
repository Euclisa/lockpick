#ifndef _LOCKPICK_FA_H
#define _LOCKPICK_FA_H

#include <lockpick/htable.h>


typedef struct lp_fa_state
{

} lp_regex_fa_state_t;


typedef struct lp_nfa
{
    lp_regex_fa_state_t *start;
    lp_regex_fa_state_t **accept;
    lp_htable_t *transitions;
} lp_regex_nfa_t;

#endif // _LOCKPICK_REGEX_FA_H