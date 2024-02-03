#include <stdlib.h>
#include <lockpick/slab.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>


/**
 * __lp_slab_block_list_remove - removes free block entry 'entry' from the list
 * @head:       pointer on address where current free blocks list's head is located
 * @entry:       pointer on the entry to be removed
 * 
 * Returns true on success, false on failure.
 * 
 * Wrapper function for general list api 'lp_list_remove'.
*/
static inline bool __lp_slab_block_list_remove(__lp_slab_block_list_t **head, __lp_slab_block_list_t *entry)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_remove(&old_head,&entry->__node))
        return_set_errno(false,EINVAL);

    *head = __lp_slab_block_list_container(old_head);

    return true;
}


/**
 * __lp_slab_block_list_insert_before - inserts free block entry 'entry' before 'position'
 * @head:           pointer on address where current list's head is located
 * @position:       pointer on an entry before which to perform insertion
 * @entry:          pointer on the entry to be inserted before 'position'
 * 
 * Returns true on success, false on failure.
 * 
 * Wrapper function for general list api 'lp_list_insert_before'.
*/
static inline bool __lp_slab_block_list_insert_before(__lp_slab_block_list_t **head, __lp_slab_block_list_t *position, __lp_slab_block_list_t *entry)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_insert_before(&old_head,&position->__node,&entry->__node))
        return_set_errno(false,EINVAL);
    
    *head = __lp_slab_block_list_container(old_head);

    return true;
}


/**
 * __lp_slab_block_list_insert_after - inserts free block entry 'entry' after 'position'
 * @head:           pointer on address where current list's head is located
 * @position:       pointer on an entry after which to perform insertion
 * @entry:          pointer on the entry to be inserted after 'position'
 * 
 * Returns true on success, false on failure.
 * 
 * Wrapper function for general list api 'lp_list_insert_after'.
*/
static inline bool __lp_slab_block_list_insert_after(__lp_slab_block_list_t **head, __lp_slab_block_list_t *position, __lp_slab_block_list_t *entry)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_insert_after(&old_head,&position->__node,&entry->__node))
        return_set_errno(false,EINVAL);
    
    *head = __lp_slab_block_list_container(old_head);

    return true;
}


/**
 * __lp_slab_block_list_push_back - inserts free block entry 'entry' at the end of the list
 * @head:           pointer on address where current list's head is located
 * @entry:          pointer on the entry to be inserted at the end
 * 
 * Returns true on success, false on failure.
 * 
 * Wrapper function for general list api 'lp_list_push_back'.
*/
static inline bool __lp_slab_block_list_push_back(__lp_slab_block_list_t **head, __lp_slab_block_list_t *entry)
{
    lp_list_t *old_head = *head ? &(*head)->__node : NULL;
    if(!lp_list_push_back(&old_head,&entry->__node))
        return_set_errno(false,EINVAL);
    
    *head = __lp_slab_block_list_container(old_head);

    return true;
}


/**
 * lp_slab_create - creates new slab with given parameters
 * @total_entries:          maximum number of entries that slab can contain at once
 * @entry_size:             size of an individual entry in bytes
 * 
 * Returns pointer on created slab.
*/
lp_slab_t *lp_slab_create(size_t total_entries, size_t entry_size)
{
    lp_slab_t *slab = (lp_slab_t*)malloc(sizeof(lp_slab_t));
    affirmf(slab,"Failed to allocate memory for slab structure");

    size_t total_size = total_entries*entry_size;
    size_t alignment = MAX(__POSIX_MEMALIGN_MIN_ALIGNMENT,entry_size);
    int32_t memalign_status = posix_memalign(&slab->__buffer,alignment,total_size);
    affirmf(memalign_status == 0,"Failed to allocate memory for slab buffer of size %ld bytes",total_size);

    slab->__entry_size = entry_size;
    slab->__total_entries = slab->__total_free = total_entries;
    slab->__fb_head = (__lp_slab_block_list_t*)malloc(sizeof(__lp_slab_block_list_t));
    affirmf(slab->__fb_head,"Failed to allocate memory for slab free blocks linked list");

    slab->__fb_head->__node.next = &slab->__fb_head->__node;
    slab->__fb_head->__node.prev = &slab->__fb_head->__node;
    slab->__fb_head->__block_size = total_entries;
    slab->__fb_head->__base = (void*)slab->__buffer;

    return slab;
}


/**
 * lp_slab_release - frees memory allocated for specified slab
 * @slab:       pointer on slab instance
 * 
 * Returns nothing.
*/
void lp_slab_release(lp_slab_t *slab)
{
    affirmf(slab,"Expected valid slab pointer but null was given");
    free(slab->__buffer);
    if(slab->__fb_head)
    {
        __lp_slab_block_list_t *current_fb = __lp_slab_block_list_container(slab->__fb_head->__node.next);
        while(current_fb != slab->__fb_head)
        {
            __lp_slab_block_list_t *to_free_fb = current_fb;
            current_fb = __lp_slab_block_list_container(current_fb->__node.next);
            free(to_free_fb);
        }
    }
    free(slab->__fb_head);
    free(slab);
}


/**
 * lp_slab_alloc - allocates memory for single object withing specified slab
 * @slab:       slab object within which to perform allocation
 * 
 * Return pointer on allocated object, NULL if no space left.
*/
void *lp_slab_alloc(lp_slab_t *slab)
{
    affirmf(slab,"All nodes must be allocated within the appropriate graph slab, but null slab pointer was given");

    if(!slab->__fb_head)
        return_set_errno(NULL,ENOBUFS);
    
    void *ptr = slab->__fb_head->__base;
    
    if(slab->__fb_head->__block_size > 1)
    {
        --slab->__fb_head->__block_size;
        slab->__fb_head->__base += slab->__entry_size;
    }
    else
    {
        __lp_slab_block_list_t *old_head = slab->__fb_head;
        affirmf(__lp_slab_block_list_remove(&slab->__fb_head,old_head),"Failed to remove empty free block");
        free(old_head);
    }
    --slab->__total_free;

    return ptr;
}


/**
 * __lp_check_ptr_before_block - checks if given pointer is less then base of specified block
 * @ptr:                pointer to perfrom check on
 * @fb_list_entry:      pointer on general list entry which must be allocated within '__lp_slab_block_list_t'
 * 
 * Returns true if pointer is less or equal to the base of 'fb_list_entry'
*/
static inline bool __lp_check_ptr_before_block(const void *ptr, const lp_list_t *block_list_entry)
{
    __lp_slab_block_list_t *current_block = container_of(block_list_entry,__lp_slab_block_list_t,__node);

    return ptr <= current_block->__base;
}


/**
 * __lp_slab_block_list_insert_unit_before - inserts new free block with size 1 before specified entry
 * @head:           pointer on address where current free blocks list's head is located
 * @position:       free block entry before which to perfrom insertion of new block
 * @base:           pointer to the base of new block
 * 
 * Returns nothing.
*/
static inline void __lp_slab_block_list_insert_unit_before(__lp_slab_block_list_t **head, __lp_slab_block_list_t *position, void *base)
{
    __lp_slab_block_list_t *unit_block = (__lp_slab_block_list_t*)malloc(sizeof(__lp_slab_block_list_t));
    unit_block->__block_size = 1;
    unit_block->__base = base;
    affirmf(__lp_slab_block_list_insert_before(head,position,unit_block),"Failed to insert unit block before old");
}


/**
 * __lp_slab_block_list_insert_unit_after - inserts new free block with size 1 after specified entry
 * @head:           pointer on address where current free blocks list's head is located
 * @position:       free block entry after which to perfrom insertion of new block
 * @base:           pointer to the base of new block
 * 
 * Returns nothing.
*/
static inline void __lp_slab_block_list_insert_unit_after(__lp_slab_block_list_t **head, __lp_slab_block_list_t *position, void *base)
{
    __lp_slab_block_list_t *unit_block = (__lp_slab_block_list_t*)malloc(sizeof(__lp_slab_block_list_t));
    unit_block->__block_size = 1;
    unit_block->__base = base;
    affirmf(__lp_slab_block_list_insert_after(head,position,unit_block),"Failed to insert unit block after old");
}


/**
 * __lp_slab_block_list_merge_unit_diff - performs merge of two blocks with unit distance
 * @head:           pointer on address where current free blocks list's head is located
 * @first:          first block
 * @second:         second block
 * 
 * Returns nothing.
 * 
 * End of 'first' must be equal to base of 'second'.
 * CAUTION: Does not perform any check of arguments on release builds
*/
static inline void __lp_slab_block_list_merge_unit_diff(__lp_slab_block_list_t **head, __lp_slab_block_list_t *first, __lp_slab_block_list_t *second)
{
    affirmf_debug(*head != second,"Attempt to change head during adjacent list entries merge");
    affirmf_debug(first->__node.next == &second->__node && second->__node.prev == &first->__node,"Attempt to merge non-adjacent list entries");

    lp_list_t *first_l = &first->__node;
    lp_list_t *second_l = &second->__node;
    lp_list_t *old_head = &(*head)->__node;

    __lp_slab_block_list_t *merge_block = (__lp_slab_block_list_t*)malloc(sizeof(__lp_slab_block_list_t));
    merge_block->__block_size = first->__block_size + second->__block_size + 1;
    merge_block->__base = first->__base;

    affirmf(lp_list_insert_after(&old_head,&second->__node,&merge_block->__node),"Failed to insert new entry");
    affirmf(lp_list_remove(&old_head,first_l),"Failed to remove first entry");
    affirmf(lp_list_remove(&old_head,second_l),"Failed to remove second entry");

    free(first);
    free(second);

    *head = __lp_slab_block_list_container(old_head);
}


/**
 * lp_slab_free - frees address in specified slab
 * @slab:       slab instance inside which to perform deallocation
 * @ptr:        pointer on memory that should be freed.
 * 
 * Returns nothing.
 * 
 * 'ptr' must point on memory within specified slab which was allocated with 'lp_slab_alloc' call before.
*/
void lp_slab_free(lp_slab_t *slab, void *ptr)
{
    affirmf(slab,"Valid slab pointer expected but null was given");
    affirmf(ptr,"Valid entry pointer expected but null was given");
    size_t entry_size = slab->__entry_size;
    size_t total_entries = slab->__total_entries;
    void *begin_slab = slab->__buffer;
    void *end_slab = begin_slab + entry_size*total_entries;
    affirmf(ptr >= begin_slab  &&
            ptr < end_slab,
            "Provided entry pointer does not belong to the specified slab");
    affirmf((ptr-begin_slab) % entry_size == 0,"Provided entry is not aligned to slab entry size");

    ++slab->__total_free;

    if(!slab->__fb_head)
    {
        __lp_slab_block_list_t *new_fb = (__lp_slab_block_list_t*)malloc(sizeof(__lp_slab_block_list_t));
        new_fb->__block_size = 1;
        new_fb->__base = ptr;
        affirmf(__lp_slab_block_list_push_back(&slab->__fb_head,new_fb),"Failed to put free block to the empty list");
        return;
    }

    lp_list_t *current_fb_l = &slab->__fb_head->__node;

    while(!__lp_check_ptr_before_block(ptr,current_fb_l) && current_fb_l->next != &slab->__fb_head->__node)
        current_fb_l = current_fb_l->next;
    
    __lp_slab_block_list_t *current_fb = __lp_slab_block_list_container(current_fb_l);
    void *current_fb_start = current_fb->__base;
    void *current_fb_end = current_fb_start + entry_size*current_fb->__block_size;

    if(!__lp_check_ptr_before_block(ptr,current_fb_l) && current_fb_l->next == &slab->__fb_head->__node)
    {
        if(ptr == current_fb_end)
            ++current_fb->__block_size;
        else
            __lp_slab_block_list_insert_unit_after(&slab->__fb_head,current_fb,ptr);
    }
    else
    {
        __lp_slab_block_list_t *prev_fb = __lp_slab_block_list_container(current_fb_l->prev);
        void *prev_fb_start = prev_fb->__base;
        void *prev_fb_end = prev_fb_start + entry_size*prev_fb->__block_size;
        if(current_fb_start - prev_fb_end == entry_size)
            __lp_slab_block_list_merge_unit_diff(&slab->__fb_head,prev_fb,current_fb);
        else if(current_fb_start - ptr == entry_size)
        {
            current_fb->__base -= entry_size;
            ++current_fb->__block_size;
        }
        else if(ptr == prev_fb_end)
            ++prev_fb->__block_size;
        else
            __lp_slab_block_list_insert_unit_before(&slab->__fb_head,current_fb,ptr);
    }
}


void lp_slab_exec(lp_slab_t *slab, void (*callback)(void *entry_ptr, void *args), void *args)
{
    affirmf(slab,"Expected valid graph pointer but null was given");

    size_t entry_size = slab->__entry_size;
    size_t total_entries = slab->__total_entries;

    __lp_slab_block_list_t *curr_fb = slab->__fb_head;
    void *curr_entry = slab->__buffer;
    void *end_slab = slab->__buffer + total_entries*entry_size;

    if(__likely(curr_fb))
    {
        while(curr_entry != end_slab)
        {
            if(curr_entry == curr_fb->__base)
            {
                size_t fb_size = curr_fb->__block_size*entry_size;
                curr_entry += fb_size;
                curr_fb = __lp_slab_block_list_container(curr_fb->__node.next);
                continue;
            }
            
            callback(curr_entry,args);
            curr_entry += entry_size;
        }
    }
    else
    {
        for(; curr_entry != end_slab; curr_entry += entry_size)
            callback(curr_entry,args);
    }
}