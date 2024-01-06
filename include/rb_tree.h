#ifndef _LOCKPICK_INCLUDE_RB_TREE_H
#define _LOCKPICK_INCLUDE_RB_TREE_H

#include "define.h"
#include <stdbool.h>
#ifdef LOCKPICK_DEBUG
#include <assert.h>
#endif // LOCKPICK_DEBUG

#define RB_SUCCESS 0
#define RB_ALLOC_FAIL 1

enum __rb_colors
{
    __rb_black = 0,
    __rb_red   = 1
};

struct rb_node
{
    struct rb_node *left, *right;
    unsigned long __parent_color;
} aligned(2);


enum __rb_colors __rb_color(const struct rb_node *node);

void __rb_set_color(struct rb_node *node, enum __rb_colors color);

struct rb_node *rb_parent(const struct rb_node *node);
struct rb_node *rb_grandparent(const struct rb_node *node);
struct rb_node *rb_uncle(const struct rb_node *node);
void __rb_grandparent_uncle(const struct rb_node *node, struct rb_node **grandparent, struct rb_node **uncle);
struct rb_node *rb_sibling(const struct rb_node *node);

void rb_set_parent(struct rb_node *node, const struct rb_node *parent_ptr);

bool rb_is_left(const struct rb_node *node);
bool rb_is_left_p(const struct rb_node *node, const struct rb_node *parent);

bool rb_is_right(const struct rb_node *node);
bool rb_is_right_p(const struct rb_node *node, const struct rb_node *parent);

struct rb_node *rb_insert_rebalance(struct rb_node *root, struct rb_node *node);

struct rb_node *rb_remove(struct rb_node *root, struct rb_node *node);

bool rb_check_consistency(const struct rb_node *root);

#endif // LOCKPICK_INCLUDE_RB_TREE_H