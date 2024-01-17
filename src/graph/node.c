#include <lockpick/graph/node.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>


static inline bool __lpg_slab_fb_list_remove(__lpg_slab_fb_list_t **head, __lpg_slab_fb_list_t *node)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_remove(&old_head,&node->__node))
        return_set_errno(false,EINVAL);

    *head = container_of(old_head,__lpg_slab_fb_list_t,__node);

    return true;
}


static inline bool __lpg_slab_fb_list_insert_before(__lpg_slab_fb_list_t **head, __lpg_slab_fb_list_t *position, __lpg_slab_fb_list_t *node)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_insert_before(&old_head,&position->__node,&node->__node))
        return_set_errno(false,EINVAL);
    
    *head = container_of(old_head,__lpg_slab_fb_list_t,__node);

    return true;
}


static inline bool __lpg_slab_fb_list_insert_after(__lpg_slab_fb_list_t **head, __lpg_slab_fb_list_t *position, __lpg_slab_fb_list_t *node)
{
    lp_list_t *old_head = &(*head)->__node;
    if(!lp_list_insert_after(&old_head,&position->__node,&node->__node))
        return_set_errno(false,EINVAL);
    
    *head = container_of(old_head,__lpg_slab_fb_list_t,__node);

    return true;
}


lpg_graph_slab_t *lpg_create_graph_slab(size_t size)
{
    lpg_graph_slab_t *slab = (lpg_graph_slab_t*)malloc(sizeof(lpg_graph_slab_t));

    if(!slab)
        return_set_errno(NULL,ENOMEM);
    
    slab->__buffer = (lpg_node_t*)malloc(sizeof(lpg_node_t)*size);
    slab->__total_size = size;
    slab->__fb_head = (__lpg_slab_fb_list_t*)malloc(sizeof(__lpg_slab_fb_list_t));
    slab->__fb_head->__node.next = &slab->__fb_head->__node;
    slab->__fb_head->__node.prev = &slab->__fb_head->__node;
    slab->__fb_head->__block_size = size;
    slab->__fb_head->__start = (void*)slab->__buffer;

    return slab;
}


lpg_node_t *lpg_nalloc(lpg_graph_slab_t *slab)
{
    affirmf(slab,"All nodes must be allocated within the appropriate graph slab, but null slab pointer was given");

    if(!slab->__fb_head)
        return_set_errno(NULL,ENOBUFS);
    
    lpg_node_t *ptr = slab->__fb_head->__start;
    
    if(slab->__fb_head->__block_size > 1)
    {
        --slab->__fb_head->__block_size;
        slab->__fb_head->__start += sizeof(lpg_node_t);
    }
    else
    {
        __lpg_slab_fb_list_t *old_head = slab->__fb_head;
        affirmf(__lpg_slab_fb_list_remove(&slab->__fb_head,old_head),"Failed to remove empty free block");
        free(old_head);
    }

    return ptr;
}


static inline bool __lpg_check_ptr_before_block(const lpg_node_t *node_ptr, const lp_list_t *current_fb_l)
{
    __lpg_slab_fb_list_t *current_fb = container_of(current_fb_l,__lpg_slab_fb_list_t,__node);
    
    return node_ptr <= current_fb->__start;
}


static inline bool __lpg_slab_fb_list_insert_unit_before(__lpg_slab_fb_list_t **head, __lpg_slab_fb_list_t *position, lpg_node_t *node)
{
    __lpg_slab_fb_list_t *unit_fb = (__lpg_slab_fb_list_t*)malloc(sizeof(__lpg_slab_fb_list_t));
    unit_fb->__block_size = 1;
    unit_fb->__start = node;
    affirmf(__lpg_slab_fb_list_insert_after(head,position,unit_fb),"Failed to insert unit block before old");
}


static inline void __lpg_slab_fb_list_insert_unit_after(__lpg_slab_fb_list_t **head, __lpg_slab_fb_list_t *position, lpg_node_t *node)
{
    __lpg_slab_fb_list_t *unit_fb = (__lpg_slab_fb_list_t*)malloc(sizeof(__lpg_slab_fb_list_t));
    unit_fb->__block_size = 1;
    unit_fb->__start = node;
    affirmf(__lpg_slab_fb_list_insert_after(head,position,unit_fb),"Failed to insert unit block after old");
}


static inline void __lpg_slab_fb_list_merge_unit_diff(__lpg_slab_fb_list_t **head, __lpg_slab_fb_list_t *first, __lpg_slab_fb_list_t *second)
{
    #ifdef LOCKPICK_DEBUG
    lp_assert(*head != second,"Attempt to change head during adjacent list entries merge");
    lp_assert(first->__node.next == second && second->__node.prev == first,"Attempt to merge non-adjacent list entries");
    lpg_node_t *__first_end = (lpg_node_t*)first->__start + first->__block_size;
    lpg_node_t *__second_start = second->__start;
    lp_assert(__first_end-__second_start == 1,"Attempt to merge blocks with non-unit block distance");
    #endif

    lp_list_t *first_l = &first->__node;
    lp_list_t *second_l = &second->__node;
    lp_list_t *old_head = &(*head)->__node;

    __lpg_slab_fb_list_t *merge_fb = (__lpg_slab_fb_list_t*)malloc(sizeof(__lpg_slab_fb_list_t));
    merge_fb->__block_size = first->__block_size + second->__block_size + 1;
    merge_fb->__start = first->__start;
    // TODO: Implement 'lp_list_merge'
    affirmf(lp_list_insert_after(&old_head,second,&merge_fb->__node),"Failed to insert new node");
    affirmf(lp_list_remove(&old_head,first_l),"Failed to remove first node");
    affirmf(lp_list_remove(&old_head,second_l),"Failed to remove second node");
}


void lpg_nfree(lpg_graph_slab_t *slab, lpg_node_t *node_ptr)
{
    affirmf(slab,"Valid slab pointer expected but null was given");
    affirmf(node_ptr,"Valid node pointer expected but null was given");
    lpg_node_t *__end_slab = sizeof(lpg_node_t)*slab->__total_size;
    lpg_node_t *__begin_slab = slab->__buffer;
    affirmf(node_ptr >= __begin_slab  &&
            node_ptr < __end_slab &&
            (node_ptr-__begin_slab) % sizeof(lpg_node_t) == 0,
            "Provided node pointer does not belong to the specified slab");

    lp_list_t *current_fb_l = &slab->__fb_head->__node;

    while(!__lpg_check_ptr_before_block(node_ptr,current_fb_l) && current_fb_l->next != &slab->__fb_head->__node)
        current_fb_l = current_fb_l->next;
    
    __lpg_slab_fb_list_t *current_fb = container_of(current_fb_l,__lpg_slab_fb_list_t,__node);
    lpg_node_t *current_fb_start = current_fb->__start;
    lpg_node_t *current_fb_end = current_fb_start + current_fb->__block_size;

    if(!__lpg_check_ptr_before_block(node_ptr,current_fb_l) && current_fb_l->next == &slab->__fb_head->__node)
    {
        if(node_ptr == current_fb_end)
            ++current_fb->__block_size;
        else
            __lpg_slab_fb_list_insert_unit_after(&slab->__fb_head,current_fb_l,node_ptr);
    }
    else
    {
        __lpg_slab_fb_list_t *prev_fb = container_of(current_fb_l->prev,__lpg_slab_fb_list_t,__node);
        lpg_node_t *prev_fb_start = prev_fb->__start;
        lpg_node_t *prev_fb_end = prev_fb_start + prev_fb->__block_size;
        if(current_fb_start - prev_fb_end == 1)
            __lpg_slab_fb_list_merge_unit_diff(&slab->__fb_head,prev_fb,current_fb);
        else if(current_fb_start - node_ptr == 1)
        {
            --current_fb->__start;
            ++current_fb->__block_size;
        }
        else if(node_ptr == prev_fb_end)
            ++prev_fb->__block_size;
        else
            __lpg_slab_fb_list_insert_unit_before(&slab->__fb_head,current_fb_l,node_ptr);
    }
}