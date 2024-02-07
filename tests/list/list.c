#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lockpick/list.h>
#include <lockpick/container_of.h>
#include <lockpick/affirmf.h>
#include <lockpick/test.h>

#define LP_TEST_LIST_INSERT_AFTER_CODE 0
#define LP_TEST_LIST_INSERT_BEFORE_CODE 1
#define LP_TEST_LIST_PUSH_BACK_CODE 2
#define LP_TEST_LIST_PUSH_FRONT_CODE 3
#define LP_TEST_LIST_REMOVE_CODE 4

#define LP_TEST_MAX_LIST_STR_SIZE 512

#define LP_TEST_LIST_FOREACH_ELEMENTS_NUM 1000


typedef struct uint8_list
{
    lp_list_t entry;
    uint8_t value;
} uint8_list_t;


void __validate_list_case(lp_list_t *head, FILE *fp, uint32_t case_i)
{
    uint32_t value;
    uint32_t list_values_str_offset = 0;
    char list_values_str[LP_TEST_MAX_LIST_STR_SIZE];
    affirmf(fgets(list_values_str,LP_TEST_MAX_LIST_STR_SIZE,fp), "Failed to read list elements from file in case %d",case_i);
    if(strcmp(list_values_str,"empty\n"))
    {
        affirmf(sscanf(list_values_str,"%d",&value) != EOF, "Failed to read list values from file in case %d",case_i);
        uint8_list_t *head_entry = container_of(head,uint8_list_t,entry);
        LP_TEST_ASSERT(head_entry->value == value,
            "List values don't match in case %d. Expected: %d, got: %d",case_i,value,head_entry->value);
        while(list_values_str[list_values_str_offset] != ' ' && list_values_str[list_values_str_offset] != '\n')
            ++list_values_str_offset;
        if(list_values_str[list_values_str_offset] == ' ')
            ++list_values_str_offset;
        for(lp_list_t *curr_node = head->next; curr_node != head; curr_node = curr_node->next)
        {
            affirmf(sscanf(list_values_str+list_values_str_offset,"%d",&value) != EOF, "Failed to read list values from file in case %d",case_i);
            uint8_list_t *curr_node_entry = container_of(curr_node,uint8_list_t,entry);
            LP_TEST_ASSERT(curr_node_entry->value == value,
            "List values don't match in case %d. Expected: %d, got: %d",case_i,value,curr_node_entry->value);
            while(list_values_str[list_values_str_offset] != ' ' && list_values_str[list_values_str_offset] != '\n')
                ++list_values_str_offset;
            if(list_values_str[list_values_str_offset] == ' ')
                ++list_values_str_offset;
        }
    }
    else
        LP_TEST_ASSERT(!head,"List must be empty in case %d",case_i);
    lp_test_cleanup:
}


void __free_list(lp_list_t *head)
{
    if(!head)
        return;

    lp_list_t *current = head->next;
    while(current != head)
    {
        lp_list_t *to_free = current;
        current = current->next;
        free(to_free);
    }
    free(head);
}


void test_list_push_back()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_push_back.txt","r");
    
    uint32_t mode,value;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d %d\n",&mode,&value) != EOF)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = value;
        affirmf(lp_list_push_back(&head,&entry->entry),"Failed to read push back value %d in case %d",value,case_i);
        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_test_cleanup:
    __free_list(head);
}


void test_list_push_front()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_push_front.txt","r");
    
    uint32_t mode,value;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d %d\n",&mode,&value) != EOF)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = value;
        affirmf(lp_list_push_front(&head,&entry->entry),"Failed to push front value %d in case %d",value,case_i);
        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_test_cleanup:
    __free_list(head);
}


void test_list_insert_before_head()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_insert_before_head.txt","r");
    
    uint32_t mode,value,position;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d %d %d\n",&mode,&position,&value) != EOF)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = value;
        if(head)
            affirmf(lp_list_insert_before(&head,head,&entry->entry),"Failed to insert value %d before head in case %d",value,case_i);
        else
            affirmf(lp_list_push_back(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_test_cleanup:
    __free_list(head);
}


void test_list_insert_after_head()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_insert_after_head.txt","r");
    
    uint32_t mode,value,position;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d %d %d\n",&mode,&position,&value) != EOF)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = value;
        if(head)
            affirmf(lp_list_insert_after(&head,head,&entry->entry),"Failed to insert value %d after head in case %d",value,case_i);
        else
            affirmf(lp_list_push_back(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_test_cleanup:
    __free_list(head);
}


void test_list_insert_before_tail()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_insert_before_tail.txt","r");
    
    uint32_t mode,value,position;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d %d %d\n",&mode,&position,&value) != EOF)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = value;
        if(head)
            affirmf(lp_list_insert_before(&head,head->prev,&entry->entry),"Failed to insert value %d before tail in case %d",value,case_i);
        else
            affirmf(lp_list_push_back(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_test_cleanup:
    __free_list(head);
}


void test_list_insert_after_tail()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_insert_after_tail.txt","r");
    
    uint32_t mode,value,position;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d %d %d\n",&mode,&position,&value) != EOF)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = value;
        if(head)
            affirmf(lp_list_insert_after(&head,head->prev,&entry->entry),"Failed to insert value %d after tail in case %d",value,case_i);
        else
            affirmf(lp_list_push_back(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_test_cleanup:
    __free_list(head);
}


void test_list_random()
{
    FILE *fp = fopen(LOCKPICK_PROJECT_DIR "/tests/list/cases/list_random.txt","r");
    affirmf(fp, "Failed to open cases file");
    uint32_t mode,value,position;
    lp_list_t *head = NULL;
    uint32_t case_i = 0;
    while(fscanf(fp,"%d",&mode) != EOF)
    {
        lp_list_t *list_pos = NULL;
        if(mode == LP_TEST_LIST_INSERT_AFTER_CODE ||
        mode == LP_TEST_LIST_INSERT_BEFORE_CODE ||
        mode == LP_TEST_LIST_REMOVE_CODE)
        {
            affirmf(fscanf(fp,"%d",&position) != EOF, "Failed to parse position from file in case %d",case_i);
            if(head)
            {
                list_pos = head;
                for(uint32_t pos_i = 0; pos_i < position; ++pos_i)
                    list_pos = list_pos->next;
            }
        }

        uint8_list_t *entry = NULL;
        if(mode != LP_TEST_LIST_REMOVE_CODE)
        {
            affirmf(fscanf(fp,"%d",&value) != EOF, "Failed to parse value from file in case %d",case_i);
            entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
            entry->value = value;
        }
        affirmf(fscanf(fp,"\n") != EOF, "Failed to read new line character from file in case %d",case_i);

        if(head)
        {
            switch(mode)
            {
                case LP_TEST_LIST_INSERT_AFTER_CODE:
                    affirmf(lp_list_insert_after(&head,list_pos,&entry->entry),"Failed to insert value %d after tail in case %d",value,case_i);
                    break;
    
                case LP_TEST_LIST_INSERT_BEFORE_CODE:
                    affirmf(lp_list_insert_before(&head,list_pos,&entry->entry),"Failed to insert value %d after tail in case %d",value,case_i);
                    break;
    
                case LP_TEST_LIST_PUSH_BACK_CODE:
                    affirmf(lp_list_push_back(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
                    break;
    
                case LP_TEST_LIST_PUSH_FRONT_CODE:
                    affirmf(lp_list_push_front(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
                    break;
                
                case LP_TEST_LIST_REMOVE_CODE:
                    affirmf(lp_list_remove(&head,list_pos),"Failed to push back first value %d in case %d",value,case_i);
                    free(container_of(list_pos,uint8_list_t,entry));
                    break;

                default:
                    errorf("Unexpected mode code: %d",mode);
            }
        }
        else if(entry)
            affirmf(lp_list_push_back(&head,&entry->entry),"Failed to push back first value %d in case %d",value,case_i);
        else
            errorf("Attempt to insert uninitialized entry to list");

        LP_TEST_STEP_INTO(__validate_list_case(head,fp,case_i));
        ++case_i;
    }
    lp_list_t *old_head = head;
    affirmf(lp_list_remove(&head,head),"Failed to remove last element from list",value,case_i);
    LP_TEST_ASSERT(!head,"List is not empty after last element removal");
    free(container_of(old_head,uint8_list_t,entry));

    lp_test_cleanup:
    __free_list(head);
}


void test_list_foreach()
{
    lp_list_t *head = NULL;
    uint8_t entries_values[LP_TEST_LIST_FOREACH_ELEMENTS_NUM] = {0};
    srand(1);
    for(uint32_t entry_i = 0; entry_i < LP_TEST_LIST_FOREACH_ELEMENTS_NUM; ++entry_i)
    {
        uint8_list_t *entry = (uint8_list_t*)malloc(sizeof(uint8_list_t));
        entry->value = entries_values[entry_i] = rand() % 255;
        affirmf(lp_list_push_back(&head,&entry->entry),"Failed to insert entry on init phase.");
    }

    uint32_t entry_i = 0;
    lp_list_foreach(head,entry,uint8_list_t,entry)
    {
        LP_TEST_ASSERT(entry->value == entries_values[entry_i],"Values don't match");
        ++entry_i;
    }
    
    lp_list_t *empty_head = NULL;
    lp_list_foreach(empty_head,entry,uint8_list_t,entry)
        LP_TEST_ASSERT(false,"Head is empty but entered foreach body");

    lp_test_cleanup:
    __free_list(head);
}


void lp_test_list()
{
    LP_TEST_RUN(test_list_push_back());
    LP_TEST_RUN(test_list_push_front());
    LP_TEST_RUN(test_list_insert_before_head());
    LP_TEST_RUN(test_list_insert_after_head());
    LP_TEST_RUN(test_list_insert_before_tail());
    LP_TEST_RUN(test_list_insert_after_tail());
    LP_TEST_RUN(test_list_foreach());
    LP_TEST_RUN(test_list_random());
}