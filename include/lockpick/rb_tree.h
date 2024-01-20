#ifndef _LOCKPICK_RB_TREE_H
#define _LOCKPICK_RB_TREE_H

#include <stdbool.h>
#include <stdint.h>
#include <lockpick/define.h>


typedef enum __lp_rb_colors
{
    __LP_RB_BLACK = 0,
    __LP_RB_RED   = 1
} __lp_rb_color_t;

typedef struct lp_rb_node
{
    struct lp_rb_node *left, *right;
    uintptr_t __parent_color;
} aligned(2) lp_rb_node_t;


__lp_rb_color_t __lp_rb_color(const lp_rb_node_t *node);

void __lp_rb_set_color(lp_rb_node_t *node, __lp_rb_color_t color);

lp_rb_node_t *lp_rb_parent(const lp_rb_node_t *node);
lp_rb_node_t *lp_rb_grandparent(const lp_rb_node_t *node);
lp_rb_node_t *lp_rb_uncle(const lp_rb_node_t *node);
lp_rb_node_t *lp_rb_sibling(const lp_rb_node_t *node);

void lp_rb_set_parent(lp_rb_node_t *node, const lp_rb_node_t *parent_ptr);

bool lp_rb_is_left(const lp_rb_node_t *node);
bool lp_rb_is_left_p(const lp_rb_node_t *node, const lp_rb_node_t *parent);

bool lp_rb_is_right(const lp_rb_node_t *node);
bool lp_rb_is_right_p(const lp_rb_node_t *node, const lp_rb_node_t *parent);

lp_rb_node_t *lp_rb_insert_rebalance(lp_rb_node_t *root, lp_rb_node_t *node);

lp_rb_node_t *lp_rb_remove(lp_rb_node_t *root, lp_rb_node_t *node);

bool lp_rb_check_consistency(const lp_rb_node_t *root);

#endif // _LOCKPICK_RB_TREE_H