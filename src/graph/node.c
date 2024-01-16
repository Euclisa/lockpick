#include <lockpick/graph/node.h>
#include <lockpick/affirmf.h>


lpg_graph_slab_t *lpg_create_graph_slab(size_t size)
{
    lpg_graph_slab_t *slab = (lpg_graph_slab_t*)malloc(sizeof(lpg_graph_slab_t));

    if(!slab)
        return_set_errno(NULL,ENOMEM);
    
    slab->__buffer = (lpg_node_t*)malloc(sizeof(lpg_node_t)*size);
    slab->__fb_head = (__lpg_slab_fb_list_t*)malloc(sizeof(__lpg_slab_fb_list_t));
    slab->__fb_head->__node.next = &slab->__fb_head->__node;
    slab->__fb_head->__node.prev = &slab->__fb_head->__node;
    slab->__fb_head->__block_size = size;
    slab->__fb_head->__start = (void*)slab->__buffer;

    return slab;
}


lpg_node_t *lpg_nalloc(lpg_graph_slab_t *slab)
{
    affirmf(slab,"All nodes must be allocated within appropriate graph slab, but null slab ptr was given");

    if(!slab->__fb_head)
        return_set_errno(NULL,ENOBUFS);
    
    if(slab->__fb_head->__block_size > 1)
    {
        --slab->__fb_head->__block_size;
        slab->__fb_head->__start += sizeof(lpg_node_t);
    }
    else
    {
        lp_list_t *fb_head = slab->__fb_head;
        lp_list_remove(&fb_head,fb_head);
    }
}