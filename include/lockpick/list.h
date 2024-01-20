#ifndef _LOCKPICK_LIST_H
#define _LOCKPICK_LIST_H

#include <lockpick/define.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct lp_list
{
    struct lp_list *prev;
    struct lp_list *next;
} lp_list_t;

#define LP_LIST_HEAD_INIT(name) { &(name), &(name) }
#define LP_LIST_HEAD(name) lp_list_t name = LP_LIST_HEAD_INIT(name)

bool lp_list_insert_before(lp_list_t **head, lp_list_t *position, lp_list_t *entry);
bool lp_list_insert_after(lp_list_t **head, lp_list_t *position, lp_list_t *entry);

bool lp_list_push_back(lp_list_t **head, lp_list_t *entry);
bool lp_list_push_front(lp_list_t **head, lp_list_t *entry);

bool lp_list_remove(lp_list_t **head, lp_list_t *entry);

#define lp_list_foreach(head,entry,entry_type,entry_node_memeber)                                       \
        for(entry_type *entry = (head ? container_of(head,entry_type,entry_node_memeber) : NULL);       \
        entry && (!((uintptr_t)entry & 1) || ((entry = (lp_list_t*)((uintptr_t)entry & ~1)) != head));  \
        entry = (lp_list_t*)((uintptr_t)entry->entry_node_memeber.next | 1))

#endif // _LOCKPICK_LP_TEST_H