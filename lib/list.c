#include <lockpick/list.h>
#include <lockpick/affirmf.h>

#define __lp_list_valid_head(head)  \
        affirmf_debug((head)->next->prev == (head) && (head)->prev->next == (head), "Invalid head")

#define __lp_list_valid_adj_nodes(first,second)  \
        affirmf_debug((first) && (second) && (first)->next == second && (second)->prev == first, "Invalid adjacent nodes")


/**
 * __lp_list_insert_between - inserts 'entry' between first and second.
 * @first:      pointer on the first list entry
 * @second:     pointer on the second list entry
 * @entry:       pointer on the entry to be inserted between
 * 
 * Returns nothing.
 * 
 * Also works on lists with one entry if head is passed to first two arguments.
 * CAUTION: Does not perform any checks on release builds.
*/
static inline void __lp_list_insert_between(lp_list_t *first, lp_list_t *second, lp_list_t *entry)
{
    __lp_list_valid_adj_nodes(first,second);

    entry->prev = first;
    entry->next = second;
    first->next = second->prev = entry;
}


/**
 * lp_list_insert_before - inserts new entry 'entry' before 'position'
 * @head:       pointer on address where current list's head is located
 * @position:   pointer on an entry before which to perform insertion
 * @entry:       pointer on the entry to be inserted before 'position'
 * 
 * Returns true on success, false on failure.
 * 
 * 'head' can't point on NULL address (before operation list must contain at least one element).
 * List's head can be modified in case 'position' points on it.
*/
inline bool lp_list_insert_before(lp_list_t **head, lp_list_t *position, lp_list_t *entry)
{
    if(!head || !(*head) || !position || !entry)
        return_set_errno(false,EINVAL);
    
    __lp_list_insert_between(position->prev,position,entry);

    if(position == *head)
        (*head) = entry;

    return true;
}


/**
 * lp_list_insert_after - inserts new entry 'entry' after 'position'
 * @head:       pointer on address where current list's head is located
 * @position:   pointer on an entry after which to perform insertion
 * @entry:       pointer on the entry to be inserted after 'position'
 * 
 * Returns true on success, false on failure.
 * 
 * 'head' can't point on NULL address (before operation list must contain at least one element).
 * List's head cannot be modified in any case.
*/
inline bool lp_list_insert_after(lp_list_t **head, lp_list_t *position, lp_list_t *entry)
{
    if(!head || !(*head) || !position || !entry)
        return_set_errno(false,EINVAL);

    __lp_list_insert_between(position,position->next,entry);
    
    return true;
}


/**
 * lp_list_push_back - inserts new entry 'entry' at the end of the list
 * @head:           pointer on address where current list's head is located
 * @entry:          pointer on the entry to be inserted at the end of the list
 * 
 * Returns true on success, false on failure.
 * 
 * If 'head' points on NULL address then 'entry' becomes new head.
*/
inline bool lp_list_push_back(lp_list_t **head, lp_list_t *entry)
{
    if(!head  || !entry)
        return_set_errno(false,EINVAL);

    if(!(*head))
    {
        entry->next = entry->prev = entry;
        (*head) = entry;
    }
    else
        __lp_list_insert_between((*head)->prev,*head,entry);

    return true;
}


/**
 * lp_list_push_front - inserts new entry 'entry' at the beggining of the list
 * @head:       pointer on address where current list's head is located
 * @entry:       pointer on the entry to be inserted at the beggining of the list
 * 
 * Returns true on success, false on failure.
 * 
 * 'entry' always become new head.
*/
inline bool lp_list_push_front(lp_list_t **head, lp_list_t *entry)
{
    if(!head  || !entry)
        return_set_errno(false,EINVAL);
    
    if(!(*head))
        entry->next = entry->prev = entry;
    else
        __lp_list_insert_between((*head)->prev,*head,entry);
    (*head) = entry;

    return true;
}


/**
 * lp_list_remove - inserts entry 'entry' from the list
 * @head:       pointer on address where current list's head is located
 * @entry:       pointer on the entry to be removed
 * 
 * Returns true on success, false on failure.
 * 
 * 'head' must point on non-NULL address.
*/
inline bool lp_list_remove(lp_list_t **head, lp_list_t *entry)
{
    if(!head || !(*head) || !entry)
        return_set_errno(false,EINVAL);

    if(entry == *head)
    {
        if((*head)->next == (*head))
        {
            *head = NULL;
            return true;
        }
        *head = entry->next;
    }
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;

    return true;
}