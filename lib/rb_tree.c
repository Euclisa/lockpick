#include "rb_tree.h"
#include <stdlib.h>

#ifdef LOCKPICK_DEBUG
#define __lp_assert_rb_parent_child_ref(parent,child)  \
        lp_assert((parent->left == child) || (parent->right == child), "Node's parent must have reference to that node.");

#define __lp_assert_rb_node_color_red(node)   \
        lp_assert(__lp_rb_color(node) == __LP_RB_RED, "Node's color must be red in this case.")

#define __lp_assert_rb_node_color_black(node)   \
        lp_assert(__lp_rb_color(node) == __LP_RB_BLACK, "Node's color must be black in this case.")

#define __lp_assert_rb_node_exists(node)  \
        lp_assert(node, "Node must be present in this case.")
#endif // LOCKPICK_DEBUG


inline __lp_rb_color_t __lp_rb_color(const lp_rb_node_t *node)
{
    return node ? (__lp_rb_color_t)(node->__parent_color & 1) : __LP_RB_BLACK;
}


inline void __lp_rb_set_color(lp_rb_node_t *node, __lp_rb_color_t color)
{
    #ifdef LOCKPICK_DEBUG
    assert(node && "__lp_rb_set_color: Node must be non-null");
    #endif // LOCKPICK_DEBUG
    node->__parent_color = (node->__parent_color & ~1) | color;
}


inline lp_rb_node_t *lp_rb_parent(const lp_rb_node_t *node)
{
    return node ? (lp_rb_node_t*)(node->__parent_color & ~1) : (lp_rb_node_t *)NULL;
}


inline lp_rb_node_t *lp_rb_grandparent(const lp_rb_node_t *node)
{
    lp_rb_node_t *parent = lp_rb_parent(node);
    if(parent)
        return lp_rb_parent(parent);
    return (lp_rb_node_t*)NULL;
}


lp_rb_node_t *lp_rb_uncle(const lp_rb_node_t *node)
{
    lp_rb_node_t *parent = lp_rb_parent(node);
    if(parent)
    {
        lp_rb_node_t *grandparent = lp_rb_parent(parent);
        if(grandparent)
        {
            #ifdef LOCKPICK_DEBUG
            lp_assert(parent == grandparent->left || parent == grandparent->right, "Parent must be grandparent's child.");
            #endif // LOCKPICK_DEBUG

            if(parent == grandparent->left)
                return grandparent->right;
            else
                return grandparent->left;
        }
    }

    return (lp_rb_node_t*)NULL;
}


void __lp_rb_grandparent_uncle(const lp_rb_node_t *node, lp_rb_node_t **grandparent, lp_rb_node_t **uncle)
{
    *grandparent = NULL;
    *uncle = NULL;
    lp_rb_node_t *parent = lp_rb_parent(node);
    if(parent)
    {
        *grandparent = lp_rb_parent(parent);
        if(*grandparent)
        {
            #ifdef LOCKPICK_DEBUG
            lp_assert(parent == (*grandparent)->left || parent == (*grandparent)->right, "Parent must be grandparent's child.");
            #endif // LOCKPICK_DEBUG

            if(parent == (*grandparent)->left)
                *uncle = (*grandparent)->right;
            else
                *uncle = (*grandparent)->left;
        }
    }
}


lp_rb_node_t *lp_rb_sibling(const lp_rb_node_t *node)
{
    const lp_rb_node_t *parent = lp_rb_parent(node);
    if(!parent)
        return (lp_rb_node_t*)NULL;
    if(parent->left == node)
        return parent->right;
    else
        return parent->left;
}


void lp_rb_set_parent(lp_rb_node_t *node, const lp_rb_node_t *parent_ptr)
{
    #ifdef LOCKPICK_DEBUG
    lp_assert(!((unsigned long)parent_ptr & 1), "'parent_ptr' must be aligned at least to 2 bytes boundary.");
    #endif // LOCKPICK_DEBUG
    node->__parent_color = (unsigned long)parent_ptr | __lp_rb_color(node);
}


bool lp_rb_is_left(const lp_rb_node_t *node)
{
    lp_rb_node_t *parent = lp_rb_parent(node);
    if(!parent)
        return false;
    return parent->left == node;
}

bool lp_rb_is_left_p(const lp_rb_node_t *node, const lp_rb_node_t *parent)
{
    if(!parent)
        return false;
    return lp_rb_parent(node)->left == node;
}



bool lp_rb_is_right(const lp_rb_node_t *node)
{
    lp_rb_node_t *parent = lp_rb_parent(node);
    if(!parent)
        return false;
    return parent->right == node;
}

bool lp_rb_is_right_p(const lp_rb_node_t *node, const lp_rb_node_t *parent)
{
    if(!parent)
        return false;
    return parent->right == node;
}

/**
 * __lp_rb_rotate_left - performs rotation of the tree around specified node
 * @node:	pointer to the node around which rotation would be performed.
 * 
 */
static inline void __lp_rb_rotate_left(lp_rb_node_t *node)
{
    lp_rb_node_t *pivot = node->right;
    lp_rb_node_t *global_parent = lp_rb_parent(node);
    lp_rb_set_parent(pivot,global_parent);
    if(global_parent)
    {
        #ifdef LOCKPICK_DEBUG
        lp_assert(global_parent->left == node || global_parent->right == node, "Node must be a global_parent's child.");
        #endif

        if(global_parent->left == node)
            global_parent->left = pivot;
        else
            global_parent->right = pivot;
    }

    node->right = pivot->left;
    if(pivot->left)
        lp_rb_set_parent(pivot->left,node);
    
    pivot->left = node;
    lp_rb_set_parent(node,pivot);
}


/**
 * __lp_rb_rotate_right - performs rotation of the tree around specified node
 * @node:	pointer to the node around which rotation would be performed.
 * 
 */
static inline void __lp_rb_rotate_right(lp_rb_node_t *node)
{
    lp_rb_node_t *pivot = node->left;
    lp_rb_node_t *global_parent = lp_rb_parent(node);
    lp_rb_set_parent(pivot,global_parent);
    if(global_parent)
    {
        #ifdef LOCKPICK_DEBUG
        lp_assert(global_parent->left == node || global_parent->right == node, "Node must be a global_parent's child.");
        #endif // LOCKPICK_DEBUG

        if(global_parent->left == node)
            global_parent->left = pivot;
        else
            global_parent->right = pivot;
    }

    node->left = pivot->right;
    if(pivot->right)
        lp_rb_set_parent(pivot->right,node);
    
    pivot->right = node;
    lp_rb_set_parent(node,pivot);
}


static inline lp_rb_node_t *__lp_rb_get_left_most_child(lp_rb_node_t *root)
{
    #ifdef LOCKPICK_DEBUG
    __lp_assert_rb_node_exists(root);
    #endif // LOCKPICK_DEBUG

    while(root->left != NULL)
        root = root->left;
    return root;
}


static inline void __lp_rb_rebind_child_from_parent(const lp_rb_node_t *old_child, lp_rb_node_t *new_child, lp_rb_node_t *parent)
{
    #ifdef LOCKPICK_DEBUG
    __lp_assert_rb_parent_child_ref(parent,old_child);
    #endif // LOCKPICK_DEBUG
    
    if(parent->left == old_child)
        parent->left = new_child;
    else
        parent->right = new_child;
}


/**
 * __lp_rb_insert_rebalance_c1 - performs rebalance procedure in case when the current node is root.
 * 
 * In this case tree is empty and node is new root.
 * 
 * @root:   root of the tree.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_insert_rebalance_c1(lp_rb_node_t *root, lp_rb_node_t *node)
{
    __lp_rb_set_color(node,__LP_RB_BLACK);
    return node;
}

/**
 * __lp_rb_insert_rebalance_c2 - performs rebalance procedure in case when parent of the current node is black.
 * 
 * In this case all conditions would be met if we just color node in red.
 * 
 * @root:   root of the tree.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_insert_rebalance_c2(lp_rb_node_t *root, lp_rb_node_t *node)
{
    __lp_rb_set_color(node,__LP_RB_RED);
    return root;
}


/**
 * __lp_rb_insert_rebalance_c3 - performs rebalance procedure in case when parent and uncle of the current node are red.
 * 
 *      B                      R
 *     | \                    | \
 *    R   R          ->      B   B
 *         \                      \
 *          B*                     R*
 * 
 * @root:   root of the tree.
 * @node:	pointer to the current node.
 * @parent: pointer to the parent of the current node.
 * @uncle:  pointer to the uncle of the current node.
 * @grandparent: pointer to the grandparent of the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline void __lp_rb_insert_rebalance_c3(lp_rb_node_t *node, lp_rb_node_t *parent, lp_rb_node_t *uncle, lp_rb_node_t *grandparent)
{
    #ifdef LOCKPICK_DEBUG
    lp_assert(parent && grandparent && node && uncle, "All nodes must be non-null.");
    #endif // LOCKPICK_DEBUG
    // In this case uncle is red then parent is not root by implementation (otherwise, uncle is null -> is black)
    __lp_rb_set_color(parent,__LP_RB_BLACK);
    __lp_rb_set_color(uncle,__LP_RB_BLACK);
    __lp_rb_set_color(grandparent,__LP_RB_RED);
    __lp_rb_set_color(node,__LP_RB_RED);
}


/**
 * __lp_rb_insert_rebalance_c4 - performs rebalance procedure in case when uncle of the current node is black and parent is red
 * 
 *      B                         B                        B*
 *     | \                       | \                      | \
 *    R   B      -rotate->      R*  B      -rotate->     R   R
 *   | \                       | \ | \                  | \ | \
 *      R*                    R                                B
 * 
 * First rotate is necessary only in case of inner node.
 * 
 * @root:   root of the tree.
 * @node:	pointer to the current node.
 * @parent: pointer to the parent of the current node.
 * @grandparent: pointer to the grandparent of the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_insert_rebalance_c4(lp_rb_node_t *root, lp_rb_node_t *node, lp_rb_node_t *parent, lp_rb_node_t *grandparent)
{
    #ifdef LOCKPICK_DEBUG
    lp_assert(parent && grandparent && node, "All nodes must be non-null.");
    #endif // LOCKPICK_DEBUG
    bool node_is_right = lp_rb_is_right(node);
    bool parent_is_left = lp_rb_is_left(parent);
    __lp_rb_set_color(node,__LP_RB_RED);
    if(node_is_right && parent_is_left)
    {
        __lp_rb_rotate_left(parent);
        node = node->left;
    }
    else if(!node_is_right && !parent_is_left)
    {
        __lp_rb_rotate_right(parent);
        node = node->right;
    }

    parent = lp_rb_parent(node);
    __lp_rb_set_color(parent,__LP_RB_BLACK);
    __lp_rb_set_color(grandparent,__LP_RB_RED);
    if(parent_is_left)
        __lp_rb_rotate_right(grandparent);
    else
        __lp_rb_rotate_left(grandparent);

    return root == grandparent ? parent : root;
}


/**
 * lp_rb_insert_rebalance - performs rebalance procedure in case when parent of the current node is black
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
lp_rb_node_t *lp_rb_insert_rebalance(lp_rb_node_t *root, lp_rb_node_t *node)
{
    while(true)
    {
        lp_rb_node_t *parent = lp_rb_parent(node);
        
        // Case 1
        if(parent == NULL)
            return __lp_rb_insert_rebalance_c1(root,node);

        // Case 2
        if(__lp_rb_color(parent) == __LP_RB_BLACK)
            return __lp_rb_insert_rebalance_c2(root,node);
        
        lp_rb_node_t *grandparent;
        lp_rb_node_t *uncle;
        __lp_rb_grandparent_uncle(node,&grandparent,&uncle);
        
        // Case 4
        if(__lp_rb_color(uncle) == __LP_RB_BLACK)
            return __lp_rb_insert_rebalance_c4(root,node,parent,grandparent);
        
        // Case 3
        __lp_rb_insert_rebalance_c3(node,parent,uncle,grandparent);
        node = grandparent;
    }
}


/**
 * __lp_rb_find_relatives - finds **all** relatives, hence, one needs to provide valid pointers
 * @node:	pointer to the targer node.
 * @parent:	direct parent of the 'node'.
 * @sibling: another child of 'parent'.
 * @close_nephew: child of 'sibling' in the same direction as 'node' from 'parent'.
 * @dist_nephew: child of 'sibling' in the opposite direction as 'node' from 'parent'.
 * 
 */
static inline void __lp_rb_find_relatives(const lp_rb_node_t *node, lp_rb_node_t **parent, lp_rb_node_t **sibling, lp_rb_node_t **close_nephew, lp_rb_node_t **dist_nephew)
{
    *parent = lp_rb_parent(node);
    *sibling = NULL;
    *close_nephew = NULL;
    *dist_nephew = NULL;
    if(!*parent)
        return;
    if((*parent)->left == node)
    {
        *sibling = (*parent)->right;
        if(!*sibling)
            return;
        *close_nephew = (*sibling)->left;
        *dist_nephew = (*sibling)->right;
    }
    else
    {
        *sibling = (*parent)->left;
        if(!*sibling)
            return;
        *close_nephew = (*sibling)->right;
        *dist_nephew = (*sibling)->left;
    }
}


/**
 * __lp_rb_rebalance_leaf - rebalances tree considering that 'node' would be removed
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_rebalance_leaf(lp_rb_node_t *root, lp_rb_node_t *node)
{
    lp_rb_node_t *parent;
    lp_rb_node_t *sibling;
    lp_rb_node_t *close_nephew;
    lp_rb_node_t *dist_nephew;
    do
    {
        __lp_rb_find_relatives(node,&parent,&sibling,&close_nephew,&dist_nephew);

        if(!parent) // At first iteration parent is not null by implementation
            return root;

        #ifdef LOCKPICK_DEBUG
        __lp_assert_rb_parent_child_ref(parent,node);
        // Sibling must be, otherwise black violation would take place. Though nephews might be nil.
        __lp_assert_rb_node_exists(sibling);
        // Current node must be black by design of rb-tree.
        __lp_assert_rb_node_color_black(node);
        #endif // LOCKPICK_DEBUG

        /**
         * Parent, sibling and nephews are all black.
         * 
         *     B                  B
         *    | \                | \
         *   B*  B       ->     B*  R
         *      | \                | \
         *     B   B              B   B
         * 
         * Remember that branch (*) is short on one black node.
         */
        if
        (
            __lp_rb_color(parent) == __LP_RB_BLACK &&
            __lp_rb_color(sibling) == __LP_RB_BLACK &&
            __lp_rb_color(close_nephew) == __LP_RB_BLACK &&
            __lp_rb_color(dist_nephew) == __LP_RB_BLACK
        )
        {
            __lp_rb_set_color(sibling,__LP_RB_RED);
            node = parent;
            continue;
        }

        /**
         * Sibling is red, hence, parent and nephews are black. Rotate over parent and switch sibling-parent colors.
         * Current node has black sibling after.
         * 
         *     B                           B
         *    | \                         | \
         *   B*  R       -rotate->       R   B
         *      | \                     | \
         *     B   B                   B*  B
         * 
         * Remember that branch (*) is short on one black node.
         */
        if(__lp_rb_color(sibling) == __LP_RB_RED)
        {
            #ifdef LOCKPICK_DEBUG
            // Check for red violation
            __lp_assert_rb_node_color_black(parent);
            __lp_assert_rb_node_color_black(close_nephew);
            __lp_assert_rb_node_color_black(dist_nephew);
            // Nephews must be, otherwise black violation would take place
            __lp_assert_rb_node_exists(close_nephew);
            __lp_assert_rb_node_exists(dist_nephew);
            #endif // LOCKPICK_DEBUG

            if(root == parent)
                root = sibling;

            __lp_rb_set_color(parent,__LP_RB_RED);
            __lp_rb_set_color(sibling,__LP_RB_BLACK);
            if(lp_rb_is_left_p(node,parent))
                __lp_rb_rotate_left(parent);
            else
                __lp_rb_rotate_right(parent);
            __lp_rb_find_relatives(node,&parent,&sibling,&close_nephew,&dist_nephew);
        }

        /**
         * Parent is red, sibling and nephews are black.
         * 
         *     R                  B
         *    | \                | \
         *   B*  B       ->     B*  R
         *      | \                | \
         *     B   B              B   B
         * 
         * Remember that branch (*) is short on one black node.
         */
        if
        (
            __lp_rb_color(parent) == __LP_RB_RED &&
            __lp_rb_color(close_nephew) == __LP_RB_BLACK &&
            __lp_rb_color(dist_nephew) == __LP_RB_BLACK
        )
        {
            #ifdef LOCKPICK_DEBUG
            // Check for red violation
            __lp_assert_rb_node_color_black(sibling);
            #endif // LOCKPICK_DEBUG

            __lp_rb_set_color(sibling,__LP_RB_RED);
            __lp_rb_set_color(parent,__LP_RB_BLACK);
            return root;
        }

        /**
         * Sibling is red, close nephew is red and distant is black.
         * In this case we make our node to have black sibling and red distant nephew.
         * 
         *     P                  P                           P
         *    | \                | \                         | \
         *   B*  B       ->     B*  R      -rotate->        B*  B
         *      | \                | \                         | \
         *     R   B              B   B                           R
         * 
         * Remember that branch (*) is short on one black node.
         */
        if
        (
            __lp_rb_color(sibling) == __LP_RB_BLACK &&
            __lp_rb_color(close_nephew) == __LP_RB_RED &&
            __lp_rb_color(dist_nephew) == __LP_RB_BLACK
        )
        {
            __lp_rb_set_color(sibling,__LP_RB_RED);
            __lp_rb_set_color(close_nephew,__LP_RB_BLACK);
            if(lp_rb_is_left_p(node,parent))
            {
                __lp_rb_rotate_right(sibling);
                dist_nephew = sibling;
                sibling = close_nephew;
                close_nephew = close_nephew->left;
            }
            else
            {
                __lp_rb_rotate_left(sibling);
                dist_nephew = sibling;
                sibling = close_nephew;
                close_nephew = close_nephew->right;
            }
        }

        #ifdef LOCKPICK_DEBUG
        // At this point distant sibling must be red by algorithm design
        __lp_assert_rb_node_color_red(dist_nephew);
        // Check for red violation
        __lp_assert_rb_node_color_black(sibling);
        // Sibling must be, otherwise black violation would take place. Though nephews might be nil.
        __lp_assert_rb_node_exists(sibling);
        #endif // LOCKPICK_DEBUG

        if(root == parent)
            root = sibling;

        /**
         * Sibling is black and distant nephew is red.
         * 
         *     RB                 B                            RB
         *    | \                | \                          | \
         *   B*  B       ->     B*  RB       -rotate->       B   B 
         *      | \                | \                      |
         *         R                  B                    B*
         * 
         * Remember that branch (*) is short on one black node.
         */

        __lp_rb_set_color(sibling,__lp_rb_color(parent));
        __lp_rb_set_color(parent,__LP_RB_BLACK);
        __lp_rb_set_color(dist_nephew,__LP_RB_BLACK);
        if(lp_rb_is_left_p(node,parent))
            __lp_rb_rotate_left(parent);
        else
            __lp_rb_rotate_right(parent);
        
        return root;

    } while(true);
}


/**
 * __lp_rb_remove_rebalance_leaf - removes node without children and rebalances tree
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_remove_rebalance_leaf(lp_rb_node_t *root, lp_rb_node_t *node)
{
    lp_rb_node_t *parent = lp_rb_parent(node);

    if(!parent)
        return NULL;

    if(__lp_rb_color(node) == __LP_RB_BLACK)
        root = __lp_rb_rebalance_leaf(root,node);

     __lp_rb_rebind_child_from_parent(node,NULL,parent);

    return root;
}


/**
 * __lp_rb_remove_node_one_child - removes node with one chilren and rebalances tree; this only child must be red
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * @non_nil_child: pointer on the only non-nil child of 'node'.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_remove_node_one_child(lp_rb_node_t *root, lp_rb_node_t *node, lp_rb_node_t *non_nil_child)
{
    #ifdef LOCKPICK_DEBUG
    // In case when node has exactly one non-nil child, this node must be black and child's color must be red. Otherwise it would be black violation.
    __lp_assert_rb_node_color_black(node);
    __lp_assert_rb_node_color_red(non_nil_child);

    lp_assert((!node->left && node->right == non_nil_child) || (!node->right && node->left == non_nil_child), "Node must have only one child non-nil child that is equal to 'non_nil_child'.");
    #endif // LOCKPICK_DEBUG

    lp_rb_node_t *parent = lp_rb_parent(node);
    lp_rb_set_parent(non_nil_child,parent);
    __lp_rb_set_color(non_nil_child,__LP_RB_BLACK);
    if(parent)
    {
        __lp_rb_rebind_child_from_parent(node,non_nil_child,parent);
        
        return root;
    }

    return non_nil_child;
}


/**
 * __lp_rb_remove_rebalance - removes node with at most one child and rebalances tree
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline lp_rb_node_t *__lp_rb_remove_rebalance(lp_rb_node_t *root, lp_rb_node_t *node)
{
    lp_rb_node_t *non_nil_child = NULL;

    #ifdef LOCKPICK_DEBUG
    lp_assert(!node->left || !node->right, "Node must have at least one nil child.");
    #endif // LOCKPICK_DEBUG

    if(node->left)
        non_nil_child = node->left;
    else if(node->right)
        non_nil_child = node->right;

    if(non_nil_child)
        return __lp_rb_remove_node_one_child(root,node,non_nil_child);
    return __lp_rb_remove_rebalance_leaf(root,node);
}


/**
 * lp_rb_remove - removes node and rebalances tree
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
lp_rb_node_t *lp_rb_remove(lp_rb_node_t *root, lp_rb_node_t *node)
{
    if(node->left != NULL && node->right != NULL)
    {
        lp_rb_node_t *successor = __lp_rb_get_left_most_child(node->right);
        lp_rb_node_t *sc_parent = lp_rb_parent(successor);
        lp_rb_node_t *sc_left = successor->left;
        lp_rb_node_t *sc_right = successor->right;
        lp_rb_node_t *node_parent = lp_rb_parent(node);
        __lp_rb_color_t sc_color = __lp_rb_color(successor);

        if(!node_parent)
            root = successor;

        __lp_rb_set_color(successor,__lp_rb_color(node));
        __lp_rb_set_color(node,sc_color);

        lp_rb_set_parent(successor,node_parent);
        if(node_parent)
            __lp_rb_rebind_child_from_parent(node,successor,node_parent);

        if(node->left)
            lp_rb_set_parent(node->left,successor);
        successor->left = node->left;
        // node->right might point on successor
        if(successor == node->right)
        {
            successor->right = node;
            lp_rb_set_parent(node,successor);
        }
        else
        {
            if(node->right)
                lp_rb_set_parent(node->right,successor);
            successor->right = node->right;
            lp_rb_set_parent(node,sc_parent);
            // successor is left child by design (if it is not a direct child of node)
            sc_parent->left = node;
        }

        if(sc_right)
            lp_rb_set_parent(sc_right,node);
        node->right = sc_right;
        // successor->left is always null by design
        node->left = sc_left;
    }

    return __lp_rb_remove_rebalance(root,node);
}


int __lp_rb_count_black_nodes_one_path(const lp_rb_node_t *root)
{
    int count = 0;
    while(root)
    {
        count += __lp_rb_color(root) == __LP_RB_BLACK ? 1 : 0;
        root = root->left;
    }
    
    return count;
}


bool __lp_rb_check_subtree_consistency(const lp_rb_node_t *root, int blacks_required)
{
    if(!root)
        return true;

    lp_rb_node_t *parent = lp_rb_parent(root);
    if(parent && parent->left != root && parent->right != root)
        return false;

    if(__lp_rb_color(root) == __LP_RB_BLACK)
        blacks_required -= 1;
    else
    {
        if(__lp_rb_color(root->left) == __LP_RB_RED || __lp_rb_color(root->right) == __LP_RB_RED)
            return false;
    }

    if(blacks_required < 0)
        return false;

    return __lp_rb_check_subtree_consistency(root->left,blacks_required) && __lp_rb_check_subtree_consistency(root->right,blacks_required);
}


bool lp_rb_check_consistency(const lp_rb_node_t *root)
{   
    int blacks_required = __lp_rb_count_black_nodes_one_path(root);

    return __lp_rb_check_subtree_consistency(root,blacks_required);
}