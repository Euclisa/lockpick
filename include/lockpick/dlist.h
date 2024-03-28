#ifndef _LOCKPICK_DLIST_H
#define _LOCKPICK_DLIST_H

#include <lockpick/define.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct lp_dlist
{
    struct lp_dlist *prev;
    struct lp_dlist *next;
} lp_dlist_t;


void lp_dlist_insert_before(lp_dlist_t **head, lp_dlist_t *position, lp_dlist_t *entry);
void lp_dlist_insert_after(lp_dlist_t **head, lp_dlist_t *position, lp_dlist_t *entry);

void lp_dlist_push_back(lp_dlist_t **head, lp_dlist_t *entry);
void lp_dlist_push_front(lp_dlist_t **head, lp_dlist_t *entry);

void lp_dlist_remove(lp_dlist_t **head, lp_dlist_t *entry);


#define lp_dlist_foreach(head,entry,entry_type,entry_node_memeber)                                                                      \
        for(entry_type *entry = ((head) ? container_of(head,entry_type,entry_node_memeber) : NULL);                                     \
        entry && (!((uintptr_t)entry & 1) || (&(entry = (entry_type*)((uintptr_t)entry & ~1))->entry_node_memeber != (head)));          \
        entry = (entry_type*)((uintptr_t)(container_of(entry->entry_node_memeber.next,entry_type,entry_node_memeber)) | 1))

#define lp_dlist_foreach_rev(head,entry,entry_type,entry_node_memeber)                                                                  \
        for(entry_type *entry = ((head) ? container_of(head,entry_type,entry_node_memeber) : NULL);                                     \
        entry && (!((uintptr_t)entry & 1) || (&(entry = (entry_type*)((uintptr_t)entry & ~1))->entry_node_memeber != (head)));          \
        entry = (entry_type*)((uintptr_t)(container_of(entry->entry_node_memeber.prev,entry_type,entry_node_memeber)) | 1))

#endif // _LOCKPICK_DLIST_H