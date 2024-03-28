#include <lockpick/dlist.h>
#include <lockpick/affirmf.h>

#define __lp_dlist_valid_head(head)  \
        affirmf_debug((head)->next->prev == (head) && (head)->prev->next == (head), "Invalid head")

#define __lp_dlist_valid_adj_nodes(first,second)  \
        affirmf_debug((first) && (second) && (first)->next == second && (second)->prev == first, "Invalid adjacent nodes")


/**
 * __lp_dlist_insert_between - inserts 'entry' between first and second.
 * @first:      pointer on the first list entry
 * @second:     pointer on the second list entry
 * @entry:       pointer on the entry to be inserted between
 * 
 * Returns nothing.
 * 
 * Also works on lists with one entry if head is passed to first two arguments.
 * CAUTION: Does not perform any checks on release builds.
*/
static inline void __lp_dlist_insert_between(lp_dlist_t *first, lp_dlist_t *second, lp_dlist_t *entry)
{
    __lp_dlist_valid_adj_nodes(first,second);

    entry->prev = first;
    entry->next = second;
    first->next = second->prev = entry;
}


/**
 * lp_dlist_insert_before - inserts new node into linked list before specified node
 * @head:       pointer to head node address 
 * @position:   pointer to node before which to insert
 * @entry:      pointer to new node to insert
 *
 * Inserts the node pointed to by @entry into the linked list before the 
 * node pointed to by @position.
 * 
 * CAUTION: @head must not be NULL, hence, list must already contain at least one node.
 *  
 * Updates the head pointer if 'position' points to head node. Terminates 
 * program on any error.
 *
 * Returns nothing.
*/
inline void lp_dlist_insert_before(lp_dlist_t **head, lp_dlist_t *position, lp_dlist_t *entry)
{
    affirm_nullptr(head,"list head");
    affirmf(*head,"List must not be empty for this operation");
    affirm_nullptr(position,"position");
    affirm_nullptr(entry,"entry");
    
    __lp_dlist_insert_between(position->prev,position,entry);

    if(position == *head)
        (*head) = entry;
}


/**
 * lp_dlist_insert_after - inserts new node into linked list after specified node
 * @head:       pointer to head node address 
 * @position:   pointer to node after which to insert
 * @entry:      pointer to new node to insert
 *
 * Inserts the node pointed to by @entry into the linked list before the 
 * node pointed to by @position.
 * 
 * CAUTION: @head must not be NULL, hence, list must already contain at least one node.
 *
 * Returns nothing.
*/
inline void lp_dlist_insert_after(lp_dlist_t **head, lp_dlist_t *position, lp_dlist_t *entry)
{
    affirm_nullptr(head,"list head");
    affirmf(*head,"List must not be empty for this operation");
    affirm_nullptr(position,"position");
    affirm_nullptr(entry,"entry");

    __lp_dlist_insert_between(position,position->next,entry);
}


/**
 * lp_dlist_push_back - inserts new entry at the end of the list
 * @head:           pointer on address where current list's head is located
 * @entry:          pointer on the entry to be inserted at the end of the list
 * 
 * If @head points on NULL address then 'entry' becomes new head.
 * 
 * Returns nothing.
*/
inline void lp_dlist_push_back(lp_dlist_t **head, lp_dlist_t *entry)
{
    affirm_nullptr(head,"list head");
    affirm_nullptr(entry,"entry");

    if(!(*head))
    {
        entry->next = entry->prev = entry;
        *head = entry;
    }
    else
        __lp_dlist_insert_between((*head)->prev,*head,entry);
}


/**
 * lp_dlist_push_front - inserts new entry at the beggining of the list
 * @head:       pointer on address where current list's head is located
 * @entry:      pointer on the entry to be inserted at the beggining of the list
 * 
 * @entry always become new head.
 * 
 * Returns nothing.
*/
inline void lp_dlist_push_front(lp_dlist_t **head, lp_dlist_t *entry)
{
    affirm_nullptr(head,"list head");
    affirm_nullptr(entry,"entry");
    
    if(!(*head))
        entry->next = entry->prev = entry;
    else
        __lp_dlist_insert_between((*head)->prev,*head,entry);
    *head = entry;
}


/**
 * lp_dlist_remove - removes entry from the list
 * @head:       pointer on address where current list's head is located
 * @entry:      pointer on the entry to be removed
 * 
 *  @head must point on non-NULL address.
 * 
 * Returns nothing.
*/
inline void lp_dlist_remove(lp_dlist_t **head, lp_dlist_t *entry)
{
    affirm_nullptr(head,"list head");
    affirmf(*head,"List must not be empty for this operation");
    affirm_nullptr(entry,"entry");

    if(entry == *head)
    {
        if((*head)->next == (*head))
        {
            *head = NULL;
            return;
        }
        *head = entry->next;
    }
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
}