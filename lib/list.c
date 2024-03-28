#include <lockpick/list.h>
#include <lockpick/affirmf.h>


void lp_list_insert_after(lp_list_t *pos, lp_list_t *entry)
{
    affirm_nullptr(pos,"position");
    affirm_nullptr(entry,"entry");

    entry->next = pos->next;
    pos->next = entry;
}


void lp_list_push_head(lp_list_t **head, lp_list_t *entry)
{
    affirm_nullptr(head,"pointer on head location");
    affirm_nullptr(entry,"entry");

    entry->next = *head;
    *head = entry;
}


void lp_list_remove(lp_list_t **head, lp_list_t *entry, lp_list_t *prev)
{
    affirm_nullptr(head,"pointer on head location");
    affirm_nullptr(entry,"entry");

    if(entry == *head)
        *head = entry->next;
    else
    {
        affirm_nullptr(prev,"previous entry");
        prev->next = entry->next;
    }
}


void lp_list_remove_head(lp_list_t **head)
{
    affirm_nullptr(head,"pointer on head location");
    affirm_nullptr(*head,"head");

    *head = (*head)->next;
}