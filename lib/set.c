#include <lockpick/set.h>
#include <lockpick/affirmf.h>
#include <lockpick/container_of.h>
#include <malloc.h>
#include <string.h>


/**
 * lp_set_create - creates new set instance
 * @entry_size:     size of stored data
 * @ls:             comparison 'less' function
 * 
 * Creates red-black tree based set for storing elements of size @entry_size with 
 * defined relation operator @ls that returns true if first argument
 * is less than second.
 * 
 * Set does not store pointers to objects but copies.
 * 
 * Returns: pointer to set object
*/
lp_set_t *lp_set_create(size_t entry_size, bool (*ls)(const void *, const void *))
{
    affirm_nullptr(ls,"compare 'less' function poiinter");

    size_t set_size = sizeof(lp_set_t);
    lp_set_t *set = (lp_set_t*)malloc(set_size);
    affirm_bad_malloc(set,"set",set_size);

    set->__rb_root = NULL;
    set->__entry_size = entry_size;
    set->__size = 0;
    set->__ls = ls;

    return set;
}


static inline void __lp_set_release_entry(lp_rb_node_t *node)
{
    lp_set_entry_t *entry = container_of(node,lp_set_entry_t,__rb_node);
    free(entry->__data);
    free(entry);
}


static void __lp_set_release_rb_tree(lp_rb_node_t *root)
{
    if(root->left)
        __lp_set_release_rb_tree(root->left);
    if(root->right)
        __lp_set_release_rb_tree(root->right);
    
    __lp_set_release_entry(root);
}


/**
 * lp_set_release - releases set instance
 * @set:        set object to be released
 * 
 * Releases underlying red-black tree, data elements
 * and set structure itself.
 * 
 * Returns: nothing
*/
void lp_set_release(lp_set_t *set)
{
    affirm_nullptr(set,"set");

    if(set->__rb_root)
        __lp_set_release_rb_tree(set->__rb_root);
    free(set);
}


/**
 * __lp_set_rb_find_node - finds node with given data
 * @set:                set object
 * @data:               element to be found
 * @res_node:           pointer to pointer to memory location where target node resides
 * @res_node_parent:    pointer to target node parent
 * 
 * Finds node inside underlying rb-tree which contains data element equal to @data.
 * If no such node was found, @res_node would contain location where such node should be.
 * 
 * Equality of 'a' and 'b' is determined as "!(a < b) and !(a > b)".
 * 
 * Memory location of target node is copied to @res_node. This is convenient when target node
 * was not found and one can use this location to put there newly created node and bind it with
 * @res_node_parent using standard rb-tree API.
 * 
 * Returns: nothing
*/
static inline void __lp_set_rb_find_node(lp_set_t *set, const void *data, lp_rb_node_t ***res_node, lp_rb_node_t **res_node_parent)
{
    lp_rb_node_t **curr_node = &set->__rb_root;
    lp_rb_node_t *prev_node = NULL;
    while(*curr_node)
    {
        lp_set_entry_t *curr_entry_node = container_of(*curr_node,lp_set_entry_t,__rb_node);
        prev_node = *curr_node;
        if(set->__ls(curr_entry_node->__data,data))
            curr_node = &(*curr_node)->right;
        else if(set->__ls(data,curr_entry_node->__data))
            curr_node = &(*curr_node)->left;
        else
            break;
    }

    *res_node = curr_node;
    *res_node_parent = prev_node;
}


/**
 * lp_set_insert - inserts data element to set
 * @set:        set object
 * @data:       data to be inserted
 * 
 * Inserts @data to @set, rebalancing underlying rb-tree afterwards.
 * 
 * Returns: 'true' if element was inserted, 'false' if such element is already present
*/
bool lp_set_insert(lp_set_t *set, void *data)
{
    affirm_nullptr(set,"set");
    affirm_nullptr(data,"data");

    lp_rb_node_t **curr_node;
    lp_rb_node_t *prev_node;
    __lp_set_rb_find_node(set,data,&curr_node,&prev_node);

    if(*curr_node)
        return false;

    size_t entry_node_size = sizeof(lp_set_entry_t);
    lp_set_entry_t *entry_node = (lp_set_entry_t*)malloc(entry_node_size);
    affirm_bad_malloc(entry_node,"entry node",entry_node_size);
    
    entry_node->__rb_node.left = NULL;
    entry_node->__rb_node.right = NULL;
    lp_rb_set_parent(&entry_node->__rb_node,prev_node);

    entry_node->__data = malloc(set->__entry_size);
    affirm_bad_malloc(entry_node->__data,"entry node data",set->__entry_size);
    memcpy(entry_node->__data,data,set->__entry_size);
    
    *curr_node = &entry_node->__rb_node;

    set->__rb_root = lp_rb_insert_rebalance(set->__rb_root,*curr_node);
    ++set->__size;

    return true;
}


/**
 * lp_set_find - finds element in a set
 * @set:        set object
 * @data:       data to be found
 * @result:     pointer found data to be stored in
 * 
 * Tries to find @data inside @set. If found and @result is not NULL then
 * copies found element inside @result.
 * 
 * Returns: 'true' if element was found, 'false' otherwise
*/
bool lp_set_find(lp_set_t *set, const void *data, void *result)
{
    affirm_nullptr(set,"set");
    affirm_nullptr(data,"data");

    lp_rb_node_t **curr_node;
    lp_rb_node_t *prev_node;
    __lp_set_rb_find_node(set,data,&curr_node,&prev_node);

    if(!(*curr_node))
        return false;
    
    if(result)
    {
        lp_set_entry_t *curr_node_entry = container_of(*curr_node,lp_set_entry_t,__rb_node);
        memcpy(result,curr_node_entry->__data,set->__entry_size);
    }

    return true;
}


/**
 * lp_set_remove - removes element from set
 * @set:        set object
 * @data:       data to be removed
 * 
 * Tries to remove element equal to @data from @set.
 * 
 * Returns: 'true' if element was removed, 'false' if no such element found
*/
bool lp_set_remove(lp_set_t *set, const void *data)
{
    affirm_nullptr(set,"set");
    affirm_nullptr(data,"data");
    affirmf(set->__size > 0,"Set is empty");

    lp_rb_node_t **curr_node;
    lp_rb_node_t *prev_node;
    __lp_set_rb_find_node(set,data,&curr_node,&prev_node);

    if(!(*curr_node))
        return false;

    lp_rb_node_t *deleted_node = *curr_node;
    set->__rb_root = lp_rb_remove(set->__rb_root,deleted_node);
    --set->__size;

    __lp_set_release_entry(deleted_node);

    return true;
}


/**
* lp_set_begin - get the first entry in the set
* @set:     set object
*
* Returns the first entry in @set, according to the defined relation operator.
* If @set is empty, returns NULL.
*
* The returned entry is the one that resides within the underlying red-black
* tree of @set. It should be used only for read-only access to its data field
* and for iterating over the set elements.
*
* If the entry is removed after being returned, any further operations on it
* will result in undefined behavior.
*
* Returns: A constant pointer to the first entry in the set, or NULL if the
*         set is empty.
*/
const lp_set_entry_t *lp_set_begin(const lp_set_t *set)
{
    affirm_nullptr(set,"set");

    if(set->__size == 0)
        return NULL;

    lp_rb_node_t *curr_node = set->__rb_root;
    while(curr_node->left)
        curr_node = curr_node->left;
    
    return container_of(curr_node,lp_set_entry_t,__rb_node);
}


/**
* lp_set_end - get the last entry in the set
* @set:     set object
*
* Returns the last entry in @set, according to the defined relation operator.
* If @set is empty, returns NULL.
*
* The returned entry is the one that resides within the underlying red-black
* tree of @set. It should be used only for read-only access to its data field
* and for iterating over the set elements.
*
* If the entry is removed after being returned, any further operations on it
* will result in undefined behavior.
*
* Returns: A constant pointer to the last entry in the set, or NULL if the
*         set is empty.
*/
const lp_set_entry_t *lp_set_end(const lp_set_t *set)
{
    affirm_nullptr(set,"set");

    if(set->__size == 0)
        return NULL;

    lp_rb_node_t *curr_node = set->__rb_root;
    while(curr_node->right)
        curr_node = curr_node->right;
    
    return container_of(curr_node,lp_set_entry_t,__rb_node);
}


/**
* lp_set_next - get the next entry in the set
* @entry:       the current entry
*
* Returns the entry after @entry in the set. If @entry is the last entry
* in the set, returns NULL.
*
* Returns: a constant pointer to the next entry, or NULL if @entry is the
*         last entry.
*/
const lp_set_entry_t *lp_set_next(const lp_set_entry_t *entry)
{
    affirm_nullptr(entry,"entry");

    const lp_rb_node_t *curr_node = &entry->__rb_node;
    if(curr_node->right)
    {
        curr_node = curr_node->right;
        while(curr_node->left)
            curr_node = curr_node->left;

        return container_of(curr_node,lp_set_entry_t,__rb_node);
    }
    
    const lp_rb_node_t *curr_node_parent = lp_rb_parent(curr_node);
    while(lp_rb_is_right_p(curr_node,curr_node_parent))
    {
        curr_node = curr_node_parent;
        curr_node_parent = lp_rb_parent(curr_node);
    }

    return curr_node_parent ? container_of(curr_node_parent,lp_set_entry_t,__rb_node) : NULL;
}


/**
* lp_set_next - get the previous entry in the set
* @entry:       the current entry
*
* Returns the entry before @entry in the set. If @entry is the first entry
* in the set, returns NULL.
*
* Returns: a constant pointer to the previous entry, or NULL if @entry is the
*         first entry.
*/
const lp_set_entry_t *lp_set_prev(const lp_set_entry_t *entry)
{
    affirm_nullptr(entry,"entry");

    const lp_rb_node_t *curr_node = &entry->__rb_node;
    if(curr_node->left)
    {
        curr_node = curr_node->left;
        while(curr_node->right)
            curr_node = curr_node->right;

        return container_of(curr_node,lp_set_entry_t,__rb_node);
    }
    
    const lp_rb_node_t *curr_node_parent = lp_rb_parent(curr_node);
    while(lp_rb_is_left_p(curr_node,curr_node_parent))
    {
        curr_node = curr_node_parent;
        curr_node_parent = lp_rb_parent(curr_node);
    }

    return curr_node_parent ? container_of(curr_node_parent,lp_set_entry_t,__rb_node) : NULL;
}


/**
 * lp_set_size - get size of set
 * @set:        set object
 * 
 * Returns: number of elements in @set
*/
inline size_t lp_set_size(const lp_set_t *set)
{
    affirm_nullptr(set,"set");

    return set->__size;
}


/**
 * lp_set_is_empty - checks if set is empty
 * @set:        set object
 * 
 * Returns: 'true' if @set is empty, 'false' otherwise
*/
inline bool lp_set_is_empty(const lp_set_t *set)
{
    affirm_nullptr(set,"set");

    return set->__size == 0;
}
