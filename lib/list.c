#include <lockpick/list.h>

#ifdef LOCKPICK_DEBUG

#define __lp_list_valid_head(head)  \
        lp_assert((head)->next->prev == (head) && (head)->prev->next == (head), "Invalid head")

#define __lp_list_valid_adj_nodes(first,second)  \
        lp_assert((first) && (second) && (first)->next == second && (second)->prev == first, "Invalid adjacent nodes")

#endif // LOCKPICK_DEBUG


static inline void __lp_list_insert_between(lp_list_t *first, lp_list_t *second, lp_list_t *node)
{
    #ifdef LOCKPICK_DEBUG
    __lp_list_valid_adj_nodes(first,second);
    #endif

    node->prev = first;
    node->next = second;
    first->next = second->prev = node;
}


inline bool lp_list_insert_before(lp_list_t **head, lp_list_t *position, lp_list_t *node)
{
    if(!head || !(*head) || !position || !node)
        return_set_errno(false,EINVAL);
    
    __lp_list_insert_between(position->prev,position,node);

    if(position == *head)
        (*head) = node;

    return true;
}


inline bool lp_list_insert_after(lp_list_t **head, lp_list_t *position, lp_list_t *node)
{
    if(!head || !(*head) || !position || !node)
        return_set_errno(false,EINVAL);

    __lp_list_insert_between(position,position->next,node);
    
    return true;
}


inline bool lp_list_push_back(lp_list_t **head, lp_list_t *node)
{
    if(!head  || !node)
        return_set_errno(false,EINVAL);

    if(!(*head))
    {
        node->next = node->prev = node;
        (*head) = node;
    }
    else
        __lp_list_insert_between((*head)->prev,*head,node);

    return true;
}


inline bool lp_list_push_front(lp_list_t **head, lp_list_t *node)
{
    if(!head  || !node)
        return_set_errno(false,EINVAL);
    
    if(!(*head))
        node->next = node->prev = node;
    else
        __lp_list_insert_between((*head)->prev,*head,node);
    (*head) = node;

    return true;
}


inline bool lp_list_remove(lp_list_t **head, lp_list_t *node)
{
    if(!head || !(*head) || !node)
        return_set_errno(false,EINVAL);

    if(node == *head)
    {
        if((*head)->next == (*head))
        {
            *head = NULL;
            return true;
        }
        *head = node->next;
    }
    node->next->prev = node->prev;
    node->prev->next = node->next;

    return true;
}