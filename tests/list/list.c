#include <lockpick/list.h>
#include <lockpick/test.h>
#include <lockpick/slab/slab.h>
#include <lockpick/container_of.h>
#include <stdlib.h>
#include <stdio.h>


#define LP_TEST_LIST_ENTRIES_NUM 100


typedef struct uint32_list
{
    lp_list_t list;
    uint32_t value;
} uint32_list_t;


// Expects list with values ordered in ascending order
void __validate_list(const lp_list_t *head, uint32_t start, uint32_t end)
{
    uint32_t i = start;
    bool bad = false;
    uint32_t got = 0;
    for(const lp_list_t *curr_entry = head; curr_entry != NULL; curr_entry = curr_entry->next)
    {
        const uint32_list_t *curr_entry_cont = container_of(curr_entry,uint32_list_t,list);
        got = curr_entry_cont->value;
        bad = got != i;
        if(bad)
            break;
        ++i;
    }
    LP_TEST_ASSERT(!bad && i == end,
        "Expected: %d, got: %d with list values in range: [%d, %d)",i,got,start,end);

    lp_test_cleanup:
}


void test_list_insert_after()
{
    lp_slab_t *list_slab = lp_slab_create(LP_TEST_LIST_ENTRIES_NUM,sizeof(uint32_list_t));
    lp_list_t *head = NULL;

    uint32_list_t *head_entry = lp_slab_alloc(list_slab);
    head_entry->value = 0;
    uint32_list_t *middle_entry = lp_slab_alloc(list_slab);
    middle_entry->value = LP_TEST_LIST_ENTRIES_NUM/2;
    lp_list_push_head(&head,&middle_entry->list);
    lp_list_push_head(&head,&head_entry->list);

    lp_list_t *last_h = &head_entry->list;
    lp_list_t *last_m = &middle_entry->list;
    for(uint32_t i = 1; i < LP_TEST_LIST_ENTRIES_NUM/2; ++i)
    {
        uint32_list_t *h = lp_slab_alloc(list_slab);
        h->value = i;
        uint32_list_t *m = lp_slab_alloc(list_slab);
        m->value = LP_TEST_LIST_ENTRIES_NUM/2+i;
        lp_list_insert_after(last_h,&h->list);
        lp_list_insert_after(last_m,&m->list);
        last_h = &h->list;
        last_m = &m->list;
    }
    LP_TEST_STEP_INTO(__validate_list(head,0,LP_TEST_LIST_ENTRIES_NUM));

    lp_test_cleanup:
    lp_slab_release(list_slab);
}


void test_list_push_head()
{
    lp_slab_t *list_slab = lp_slab_create(LP_TEST_LIST_ENTRIES_NUM,sizeof(uint32_list_t));
    lp_list_t *head = NULL;

    for(uint32_t i = 0; i < LP_TEST_LIST_ENTRIES_NUM; ++i)
    {
        uint32_list_t *entry = lp_slab_alloc(list_slab);
        entry->value = LP_TEST_LIST_ENTRIES_NUM-i-1;
        lp_list_push_head(&head,&entry->list);
    }
    LP_TEST_STEP_INTO(__validate_list(head,0,LP_TEST_LIST_ENTRIES_NUM));

    lp_test_cleanup:
    lp_slab_release(list_slab);
}


void test_list_remove()
{
    uint32_list_t *list_buffer = (uint32_list_t*)malloc(LP_TEST_LIST_ENTRIES_NUM*sizeof(uint32_list_t));
    for(uint32_t i = 0; i < LP_TEST_LIST_ENTRIES_NUM; ++i)
        list_buffer[i].value = i;

    lp_list_t *head = NULL;
    for(int32_t i = LP_TEST_LIST_ENTRIES_NUM-1; i >= 0; --i)
        lp_list_push_head(&head,&list_buffer[i].list);
    
    for(int32_t i = 0; i < LP_TEST_LIST_ENTRIES_NUM/4; ++i)
        lp_list_remove(&head,&list_buffer[i].list,NULL);
    
    for(int32_t i = LP_TEST_LIST_ENTRIES_NUM/2; i < 3*(LP_TEST_LIST_ENTRIES_NUM/4); ++i)
        lp_list_remove(&head,&list_buffer[i].list,&list_buffer[LP_TEST_LIST_ENTRIES_NUM/2-1].list);
    
    for(int32_t i = LP_TEST_LIST_ENTRIES_NUM-1; i > 3*(LP_TEST_LIST_ENTRIES_NUM/4); --i)
        lp_list_remove(&head,&list_buffer[i].list,&list_buffer[i-1].list);
    
    lp_list_remove(&head,&list_buffer[3*(LP_TEST_LIST_ENTRIES_NUM/4)].list,&list_buffer[LP_TEST_LIST_ENTRIES_NUM/2-1].list);
    
    LP_TEST_STEP_INTO(__validate_list(head,LP_TEST_LIST_ENTRIES_NUM/4,LP_TEST_LIST_ENTRIES_NUM/2));

    lp_test_cleanup:
    free(list_buffer);
}


void test_list_remove_head()
{
    lp_slab_t *list_slab = lp_slab_create(LP_TEST_LIST_ENTRIES_NUM,sizeof(uint32_list_t));
    lp_list_t *head = NULL;

    for(uint32_t i = 0; i < LP_TEST_LIST_ENTRIES_NUM; ++i)
    {
        uint32_list_t *entry = lp_slab_alloc(list_slab);
        entry->value = LP_TEST_LIST_ENTRIES_NUM-i-1;
        lp_list_push_head(&head,&entry->list);
    }

    for(uint32_t i = 0; i < LP_TEST_LIST_ENTRIES_NUM; ++i)
    {
        uint32_list_t *entry = container_of(head,uint32_list_t,list);
        LP_TEST_ASSERT(entry->value == i,"Expected: %d, got %d",i,entry->value);
        lp_list_remove_head(&head);
    }

    lp_test_cleanup:
    lp_slab_release(list_slab);
}


void test_list_foreach()
{
    lp_slab_t *list_slab = lp_slab_create(LP_TEST_LIST_ENTRIES_NUM,sizeof(uint32_list_t));
    lp_list_t *head = NULL;

    for(uint32_t i = 0; i < LP_TEST_LIST_ENTRIES_NUM; ++i)
    {
        uint32_list_t *entry = lp_slab_alloc(list_slab);
        entry->value = LP_TEST_LIST_ENTRIES_NUM-i-1;
        lp_list_push_head(&head,&entry->list);
    }

    uint32_t i = 0;
    lp_list_foreach(head,entry,uint32_list_t,list)
    {
        LP_TEST_ASSERT(entry->value == i,"Expected: %d, got: %d",i,entry->value);
        ++i;
    }

    lp_test_cleanup:
    lp_slab_release(list_slab);
}


void lp_test_list()
{
    LP_TEST_RUN(test_list_insert_after());
    LP_TEST_RUN(test_list_push_head());
    LP_TEST_RUN(test_list_remove());
    LP_TEST_RUN(test_list_remove_head());
    LP_TEST_RUN(test_list_foreach());
}