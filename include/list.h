#ifndef _LOCKPICK_INCLUDE_LIST_H
#define _LOCKPICK_INCLUDE_LIST_H

#include "define.h"
#include <stdbool.h>

typedef struct lp_list
{
    struct lp_list *prev;
    struct lp_list *next;
} lp_list_t;

#define LP_LIST_HEAD_INIT(name) { &(name), &(name) }
#define LP_LIST_HEAD(name) lp_list_t name = LP_LIST_HEAD_INIT(name)

bool lp_list_insert_before(lp_list_t **head, lp_list_t *position, lp_list_t *node);
bool lp_list_insert_after(lp_list_t **head, lp_list_t *position, lp_list_t *node);

bool lp_list_push_back(lp_list_t **head, lp_list_t *node);
bool lp_list_push_front(lp_list_t **head, lp_list_t *node);

bool lp_list_remove(lp_list_t **head, lp_list_t *node);

#endif // _LOCKPICK_INCLUDE_LP_TEST_H