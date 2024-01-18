#include <stdlib.h>
#include <lockpick/slab.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>


static inline bool __lp_slab_fb_list_remove(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *node)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_remove(&old_head,&node->__node))
        return_set_errno(false,EINVAL);

    *head = container_of(old_head,__lp_slab_fb_list_t,__node);

    return true;
}


static inline bool __lp_slab_fb_list_insert_before(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *position, __lp_slab_fb_list_t *node)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_insert_before(&old_head,&position->__node,&node->__node))
        return_set_errno(false,EINVAL);
    
    *head = container_of(old_head,__lp_slab_fb_list_t,__node);

    return true;
}


static inline bool __lp_slab_fb_list_insert_after(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *position, __lp_slab_fb_list_t *node)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_insert_after(&old_head,&position->__node,&node->__node))
        return_set_errno(false,EINVAL);
    
    *head = container_of(old_head,__lp_slab_fb_list_t,__node);

    return true;
}


static inline bool __lp_slab_fb_list_push_back(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *node)
{
    lp_list_t *old_head = *head ? &(*head)->__node : NULL;
    if(!lp_list_push_back(&old_head,&node->__node))
        return_set_errno(false,EINVAL);
    
    *head = container_of(old_head,__lp_slab_fb_list_t,__node);

    return true;
}


lp_slab_t *lp_slab_create(size_t total_entries, size_t entry_size)
{
    lp_slab_t *slab = (lp_slab_t*)malloc(sizeof(lp_slab_t));
    slab->__buffer = malloc(total_entries*entry_size);
    slab->__entry_size = entry_size;
    slab->__total_entries = total_entries;
    slab->__fb_head = (__lp_slab_fb_list_t*)malloc(sizeof(__lp_slab_fb_list_t));
    slab->__fb_head->__node.next = &slab->__fb_head->__node;
    slab->__fb_head->__node.prev = &slab->__fb_head->__node;
    slab->__fb_head->__block_size = total_entries;
    slab->__fb_head->__start = (void*)slab->__buffer;

    return slab;
}


void lp_slab_release(lp_slab_t *slab)
{
    affirmf(slab,"Expected valid slab pointer but null was given");
    free(slab->__buffer);
    __lp_slab_fb_list_t *current_fb = container_of(slab->__fb_head->__node.next,__lp_slab_fb_list_t,__node);
    while(current_fb != slab->__fb_head)
    {
        __lp_slab_fb_list_t *to_free_fb = current_fb;
        current_fb = container_of(current_fb->__node.next,__lp_slab_fb_list_t,__node);
        free(to_free_fb);
    }
    free(slab->__fb_head);
    free(slab);
}


void *lp_slab_alloc(lp_slab_t *slab)
{
    affirmf(slab,"All nodes must be allocated within the appropriate graph slab, but null slab pointer was given");

    if(!slab->__fb_head)
        return_set_errno(NULL,ENOBUFS);
    
    void *ptr = slab->__fb_head->__start;
    
    if(slab->__fb_head->__block_size > 1)
    {
        --slab->__fb_head->__block_size;
        slab->__fb_head->__start += slab->__entry_size;
    }
    else
    {
        __lp_slab_fb_list_t *old_head = slab->__fb_head;
        affirmf(__lp_slab_fb_list_remove(&slab->__fb_head,old_head),"Failed to remove empty free block");
        free(old_head);
    }

    return ptr;
}


static inline bool __lp_check_ptr_before_block(const void *ptr, const lp_list_t *current_fb_l)
{
    __lp_slab_fb_list_t *current_fb = container_of(current_fb_l,__lp_slab_fb_list_t,__node);

    return ptr <= current_fb->__start;
}


static inline bool __lp_slab_fb_list_insert_unit_before(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *position, void *start)
{
    __lp_slab_fb_list_t *unit_fb = (__lp_slab_fb_list_t*)malloc(sizeof(__lp_slab_fb_list_t));
    unit_fb->__block_size = 1;
    unit_fb->__start = start;
    affirmf(__lp_slab_fb_list_insert_before(head,position,unit_fb),"Failed to insert unit block before old");
}


static inline void __lp_slab_fb_list_insert_unit_after(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *position, void *start)
{
    __lp_slab_fb_list_t *unit_fb = (__lp_slab_fb_list_t*)malloc(sizeof(__lp_slab_fb_list_t));
    unit_fb->__block_size = 1;
    unit_fb->__start = start;
    affirmf(__lp_slab_fb_list_insert_after(head,position,unit_fb),"Failed to insert unit block after old");
}


static inline void __lp_slab_fb_list_merge_unit_diff(__lp_slab_fb_list_t **head, __lp_slab_fb_list_t *first, __lp_slab_fb_list_t *second)
{
    #ifdef LOCKPICK_DEBUG
    lp_assert(*head != second,"Attempt to change head during adjacent list entries merge");
    lp_assert(first->__node.next == second && second->__node.prev == first,"Attempt to merge non-adjacent list entries");
    #endif

    lp_list_t *first_l = &first->__node;
    lp_list_t *second_l = &second->__node;
    lp_list_t *old_head = &(*head)->__node;

    __lp_slab_fb_list_t *merge_fb = (__lp_slab_fb_list_t*)malloc(sizeof(__lp_slab_fb_list_t));
    merge_fb->__block_size = first->__block_size + second->__block_size + 1;
    merge_fb->__start = first->__start;

    affirmf(lp_list_insert_after(&old_head,second,&merge_fb->__node),"Failed to insert new node");
    affirmf(lp_list_remove(&old_head,first_l),"Failed to remove first node");
    affirmf(lp_list_remove(&old_head,second_l),"Failed to remove second node");

    free(first);
    free(second);

    *head = container_of(old_head,__lp_slab_fb_list_t,__node);
}


void lp_slab_free(lp_slab_t *slab, void *ptr)
{
    affirmf(slab,"Valid slab pointer expected but null was given");
    affirmf(ptr,"Valid node pointer expected but null was given");
    size_t entry_size = slab->__entry_size;
    size_t total_entries = slab->__total_entries;
    void *begin_slab = slab->__buffer;
    void *end_slab = begin_slab + entry_size*total_entries;
    affirmf(ptr >= begin_slab  &&
            ptr < end_slab,
            "Provided node pointer does not belong to the specified slab");
    affirmf((ptr-begin_slab) % entry_size == 0,"Provided node is not aligned to slab entry size");

    if(!slab->__fb_head)
    {
        __lp_slab_fb_list_t *new_fb = (__lp_slab_fb_list_t*)malloc(sizeof(__lp_slab_fb_list_t));
        new_fb->__block_size = 1;
        new_fb->__start = ptr;
        affirmf(__lp_slab_fb_list_push_back(&slab->__fb_head,new_fb),"Failed to put free block to the empty list");
        return;
    }

    lp_list_t *current_fb_l = &slab->__fb_head->__node;

    while(!__lp_check_ptr_before_block(ptr,current_fb_l) && current_fb_l->next != &slab->__fb_head->__node)
        current_fb_l = current_fb_l->next;
    
    __lp_slab_fb_list_t *current_fb = container_of(current_fb_l,__lp_slab_fb_list_t,__node);
    void *current_fb_start = current_fb->__start;
    void *current_fb_end = current_fb_start + entry_size*current_fb->__block_size;

    if(!__lp_check_ptr_before_block(ptr,current_fb_l) && current_fb_l->next == &slab->__fb_head->__node)
    {
        if(ptr == current_fb_end)
            ++current_fb->__block_size;
        else
            __lp_slab_fb_list_insert_unit_after(&slab->__fb_head,current_fb_l,ptr);
    }
    else
    {
        __lp_slab_fb_list_t *prev_fb = container_of(current_fb_l->prev,__lp_slab_fb_list_t,__node);
        void *prev_fb_start = prev_fb->__start;
        void *prev_fb_end = prev_fb_start + entry_size*prev_fb->__block_size;
        if(current_fb_start - prev_fb_end == entry_size)
            __lp_slab_fb_list_merge_unit_diff(&slab->__fb_head,prev_fb,current_fb);
        else if(current_fb_start - ptr == entry_size)
        {
            current_fb->__start -= entry_size;
            ++current_fb->__block_size;
        }
        else if(ptr == prev_fb_end)
            ++prev_fb->__block_size;
        else
            __lp_slab_fb_list_insert_unit_before(&slab->__fb_head,current_fb_l,ptr);
    }
}