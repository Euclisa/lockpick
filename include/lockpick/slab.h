#ifndef _LOCKPICK_INCLUDE_SLAB_H
#define _LOCKPICK_INCLUDE_SLAB_H

#include <lockpick/list.h>
#include <stddef.h>

/**
 * __lpg_slab_fb_list_t - list type for slab free blocks tracking
*/
typedef struct __lp_slab_fb_list
{
    lp_list_t __node;
    size_t __block_size;
    void *__base;
} __lp_slab_fb_list_t;


typedef struct lp_slab
{
    void *__buffer;
    __lp_slab_fb_list_t *__fb_head;
    size_t __total_entries;
    size_t __entry_size;
} lp_slab_t;


#define __lp_slab_fb_list_container(ptr)    \
        container_of(ptr,__lp_slab_fb_list_t,__node)

lp_slab_t *lp_slab_create(size_t total_entries, size_t entry_size);
void lp_slab_release(lp_slab_t *slab);

void *lp_slab_alloc(lp_slab_t *slab);
void lp_slab_free(lp_slab_t *slab, void *node_ptr);


#endif // _LOCKPICK_INCLUDE_GRAPH_NODE_H