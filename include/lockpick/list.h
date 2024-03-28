#ifndef _LOCKPICK_LIST_H
#define _LOCKPICK_LIST_H


typedef struct lp_list
{
    struct lp_list *next;
} lp_list_t;


void lp_list_insert_after(lp_list_t *pos, lp_list_t *entry);

void lp_list_push_head(lp_list_t **head, lp_list_t *entry);

void lp_list_remove(lp_list_t **head, lp_list_t *entry, lp_list_t *prev);
void lp_list_remove_head(lp_list_t **head);


#define lp_list_foreach(head,entry,entry_type,entry_node_memeber)                   \
    for(entry_type *entry = container_of(head,entry_type,entry_node_memeber);       \
        entry != NULL;                                                              \
        entry = container_of(entry->list.next,entry_type,entry_node_memeber))

#endif