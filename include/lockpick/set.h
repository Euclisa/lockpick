#ifndef _LOCKPICK_SET_H
#define _LOCKPICK_SET_H

#include <lockpick/rb_tree.h>
#include <lockpick/define.h>


typedef struct lp_set_entry
{
    lp_rb_node_t __rb_node;
    void *__data;
} lp_set_entry_t;


typedef struct lp_set
{
    bool (*__ls)(const void *, const void *);
    lp_rb_node_t *__rb_root;
    size_t __entry_size;
    size_t __size;
} lp_set_t;


lp_set_t *lp_set_create(size_t entry_size, bool (*ls)(const void *, const void *));
void lp_set_release(lp_set_t *set);

bool lp_set_insert(lp_set_t *set, void *data);
bool lp_set_find(lp_set_t *set, const void *data, void *result);
bool lp_set_remove(lp_set_t *set, const void *data);

const lp_set_entry_t *lp_set_begin(const lp_set_t *set);
const lp_set_entry_t *lp_set_end(const lp_set_t *set);
const lp_set_entry_t *lp_set_next(const lp_set_entry_t *entry);
const lp_set_entry_t *lp_set_prev(const lp_set_entry_t *entry);

size_t lp_set_size(const lp_set_t *set);
bool lp_set_is_empty(const lp_set_t *set);

#endif