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


static inline enum __rb_colors __rb_color(const struct rb_node *node)
{
    return node ? (enum __rb_colors)(node->__parent_color & 1) : __rb_black;
}

static inline void __rb_set_color(struct rb_node *node, enum __rb_colors color)
{
    #ifdef LOCKPICK_DEBUG
    assert(node && "__rb_set_color: Node must be non-null");
    #endif // LOCKPICK_DEBUG
    node->__parent_color = (node->__parent_color & ~1) | color;
}

static inline struct rb_node *rb_parent(const struct rb_node *node)
{
    return node ? (struct rb_node*)(node->__parent_color & ~1) : (struct rb_node *)NULL;
}

static inline struct rb_node *rb_grandparent(const struct rb_node *node)
{
    struct rb_node *parent = rb_parent(node);
    if(parent)
        return rb_parent(parent);
    return (struct rb_node*)NULL;
}

static inline struct rb_node *rb_uncle(const struct rb_node *node)
{
    struct rb_node *parent = rb_parent(node);
    if(parent)
    {
        struct rb_node *grandparent = rb_parent(parent);
        if(grandparent)
        {
            #ifdef LOCKPICK_DEBUG
            assert(parent == grandparent->left || parent == grandparent->right && "rb_uncle: Parent must be grandparent's child.");
            #endif // LOCKPICK_DEBUG

            if(parent == grandparent->left)
                return grandparent->right;
            else
                return grandparent->left;
        }
    }

    return (struct rb_node*)NULL;
}

static inline void __rb_grandparent_uncle(const struct rb_node *node, struct rb_node **grandparent, struct rb_node **uncle)
{
    *grandparent = NULL;
    *uncle = NULL;
    struct rb_node *parent = rb_parent(node);
    if(parent)
    {
        *grandparent = rb_parent(parent);
        if(*grandparent)
        {
            #ifdef LOCKPICK_DEBUG
            assert(parent == (*grandparent)->left || parent == (*grandparent)->right && "rb_uncle: Parent must be grandparent's child.");
            #endif // LOCKPICK_DEBUG

            if(parent == (*grandparent)->left)
                *uncle = (*grandparent)->right;
            else
                *uncle = (*grandparent)->left;
        }
    }
}

static inline struct rb_node *rb_sibling(const struct rb_node *node)
{
    const struct rb_node *parent = rb_parent(node);
    if(!parent)
        return (struct rb_node*)NULL;
    if(parent->left == node)
        return parent->right;
    else
        return parent->left;
}

static inline void rb_set_parent(struct rb_node *node, const struct rb_node *parent_ptr)
{
    #ifdef LOCKPICK_DEBUG
    assert(!((unsigned long)parent_ptr & 1) && "rb_set_parent: parent_ptr must be aligned at least to 2 bytes boundary.");
    #endif // LOCKPICK_DEBUG
    node->__parent_color = (unsigned long)parent_ptr | __rb_color(node);
}

static inline bool rb_is_left(const struct rb_node *node)
{
    struct rb_node *parent = rb_parent(node);
    if(!parent)
        return false;
    return parent->left == node;
}

static inline bool rb_is_left_p(const struct rb_node *node, const struct rb_node *parent)
{
    if(!parent)
        return false;
    return rb_parent(node)->left == node;
}



static inline bool rb_is_right(const struct rb_node *node)
{
    struct rb_node *parent = rb_parent(node);
    if(!parent)
        return false;
    return parent->right == node;
}

static inline bool rb_is_right_p(const struct rb_node *node, const struct rb_node *parent)
{
    if(!parent)
        return false;
    return parent->right == node;
}

struct rb_node *rb_insert_rebalance(struct rb_node *root, struct rb_node *node);

struct rb_node *rb_remove(struct rb_node *root, struct rb_node *node);

bool rb_check_consistency(const struct rb_node *root);

#endif // LOCKPICK_INCLUDE_RB_TREE_H