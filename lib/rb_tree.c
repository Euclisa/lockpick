#include "rb_tree.h"
#include <stdlib.h>

#ifdef LOCKPICK_DEBUG
#define assert_parent_child_ref(parent,child,func_name)  \
        assert(parent->left == child || parent->right == child && func_name && "Node's parent must have reference to that node.");

#define assert_node_color_red(node,func_name)   \
        assert(__rb_color(node) == __rb_red && func_name && "Node's color must be red in this case.")

#define assert_node_color_black(node,func_name)   \
        assert(__rb_color(node) == __rb_black && func_name && "Node's color must be black in this case.")

#define assert_node_exists(node,func_name)  \
        assert(node && func_name && "Node must be present in this case.")
#endif // LOCKPICK_DEBUG


/**
 * __rb_rotate_left - performs rotation of the tree around specified node
 * @node:	pointer to the node around which rotation would be performed.
 * 
 */
static inline void __rb_rotate_left(struct rb_node *node)
{
    struct rb_node *pivot = node->right;
    struct rb_node *global_parent = rb_parent(node);
    rb_set_parent(pivot,global_parent);
    if(global_parent)
    {
        #ifdef LOCKPICK_DEBUG
        assert(global_parent->left == node || global_parent->right == node && "__rb_rotate_left: node must be a global_parent's child.");
        #endif

        if(global_parent->left == node)
            global_parent->left = pivot;
        else
            global_parent->right = pivot;
    }

    node->right = pivot->left;
    if(pivot->left)
        rb_set_parent(pivot->left,node);
    
    pivot->left = node;
    rb_set_parent(node,pivot);
}


/**
 * __rb_rotate_right - performs rotation of the tree around specified node
 * @node:	pointer to the node around which rotation would be performed.
 * 
 */
static inline void __rb_rotate_right(struct rb_node *node)
{
    struct rb_node *pivot = node->left;
    struct rb_node *global_parent = rb_parent(node);
    rb_set_parent(pivot,global_parent);
    if(global_parent)
    {
        #ifdef LOCKPICK_DEBUG
        assert(global_parent->left == node || global_parent->right == node && "__rb_rotate_right: node must be a global_parent's child.");
        #endif // LOCKPICK_DEBUG

        if(global_parent->left == node)
            global_parent->left = pivot;
        else
            global_parent->right = pivot;
    }

    node->left = pivot->right;
    if(pivot->right)
        rb_set_parent(pivot->right,node);
    
    pivot->right = node;
    rb_set_parent(node,pivot);
}


static inline struct rb_node *__rb_get_left_most_child(struct rb_node *root)
{
    #ifdef LOCKPICK_DEBUG
    assert_node_exists(root,"__rb_get_left_most_child");
    #endif // LOCKPICK_DEBUG

    while(root->left != NULL)
        root = root->left;
    return root;
}


static inline void __rb_rebind_child_from_parent(const struct rb_node *old_child, struct rb_node *new_child, struct rb_node *parent)
{
    #ifdef LOCKPICK_DEBUG
    assert_parent_child_ref(parent,old_child,"__rb_rebind_child_from_parent");
    #endif // LOCKPICK_DEBUG
    
    if(parent->left == old_child)
        parent->left = new_child;
    else
        parent->right = new_child;
}


/**
 * __rb_insert_rebalance_c1 - performs rebalance procedure in case when the current node is root.
 * 
 * In this case tree is empty and node is new root.
 * 
 * @root:   root of the tree.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline struct rb_node *__rb_insert_rebalance_c1(struct rb_node *root, struct rb_node *node)
{
    __rb_set_color(node,__rb_black);
    return node;
}

/**
 * __rb_insert_rebalance_c2 - performs rebalance procedure in case when parent of the current node is black.
 * 
 * In this case all conditions would be met if we just color node in red.
 * 
 * @root:   root of the tree.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline struct rb_node *__rb_insert_rebalance_c2(struct rb_node *root, struct rb_node *node)
{
    __rb_set_color(node,__rb_red);
    return root;
}


/**
 * __rb_insert_rebalance_c3 - performs rebalance procedure in case when parent and uncle of the current node are red.
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
static inline void __rb_insert_rebalance_c3(struct rb_node *node, struct rb_node *parent, struct rb_node *uncle, struct rb_node *grandparent)
{
    #ifdef LOCKPICK_DEBUG
    assert(parent && grandparent && node && uncle && "__rb_insert_rebalance_c3: All nodes must be non-null.");
    #endif // LOCKPICK_DEBUG
    // In this case uncle is red then parent is not root by implementation (otherwise, uncle is null -> is black)
    __rb_set_color(parent,__rb_black);
    __rb_set_color(uncle,__rb_black);
    __rb_set_color(grandparent,__rb_red);
    __rb_set_color(node,__rb_red);
}


/**
 * __rb_insert_rebalance_c4 - performs rebalance procedure in case when uncle of the current node is black and parent is red
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
static inline struct rb_node *__rb_insert_rebalance_c4(struct rb_node *root, struct rb_node *node, struct rb_node *parent, struct rb_node *grandparent)
{
    #ifdef LOCKPICK_DEBUG
    assert(parent && grandparent && node && "__rb_insert_rebalance_c4: All nodes must be non-null.");
    #endif // LOCKPICK_DEBUG
    bool node_is_right = rb_is_right(node);
    bool parent_is_left = rb_is_left(parent);
    __rb_set_color(node,__rb_red);
    if(node_is_right && parent_is_left)
    {
        __rb_rotate_left(parent);
        node = node->left;
    }
    else if(!node_is_right && !parent_is_left)
    {
        __rb_rotate_right(parent);
        node = node->right;
    }

    parent = rb_parent(node);
    __rb_set_color(parent,__rb_black);
    __rb_set_color(grandparent,__rb_red);
    if(parent_is_left)
        __rb_rotate_right(grandparent);
    else
        __rb_rotate_left(grandparent);

    return root == grandparent ? parent : root;
}


/**
 * rb_insert_rebalance - performs rebalance procedure in case when parent of the current node is black
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
struct rb_node *rb_insert_rebalance(struct rb_node *root, struct rb_node *node)
{
    while(true)
    {
        struct rb_node *parent = rb_parent(node);
        
        // Case 1
        if(parent == NULL)
            return __rb_insert_rebalance_c1(root,node);

        // Case 2
        if(__rb_color(parent) == __rb_black)
            return __rb_insert_rebalance_c2(root,node);
        
        struct rb_node *grandparent;
        struct rb_node *uncle;
        __rb_grandparent_uncle(node,&grandparent,&uncle);
        
        // Case 4
        if(__rb_color(uncle) == __rb_black)
            return __rb_insert_rebalance_c4(root,node,parent,grandparent);
        
        // Case 3
        __rb_insert_rebalance_c3(node,parent,uncle,grandparent);
        node = grandparent;
    }
}


/**
 * __rb_find_relatives - finds **all** relatives, hence, one needs to provide valid pointers
 * @node:	pointer to the targer node.
 * @parent:	direct parent of the 'node'.
 * @sibling: another child of 'parent'.
 * @close_nephew: child of 'sibling' in the same direction as 'node' from 'parent'.
 * @dist_nephew: child of 'sibling' in the opposite direction as 'node' from 'parent'.
 * 
 */
static inline void __rb_find_relatives(const struct rb_node *node, struct rb_node **parent, struct rb_node **sibling, struct rb_node **close_nephew, struct rb_node **dist_nephew)
{
    *parent = rb_parent(node);
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
 * __rb_rebalance_leaf - rebalances tree considering that 'node' would be removed
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline struct rb_node *__rb_rebalance_leaf(struct rb_node *root, struct rb_node *node)
{
    struct rb_node *parent;
    struct rb_node *sibling;
    struct rb_node *close_nephew;
    struct rb_node *dist_nephew;
    do
    {
        __rb_find_relatives(node,&parent,&sibling,&close_nephew,&dist_nephew);

        if(!parent) // At first iteration parent is not null by implementation
            return root;

        #ifdef LOCKPICK_DEBUG
        assert_parent_child_ref(parent,node,"__rb_rebalance_leaf");
        // Sibling must be, otherwise black violation would take place. Though nephews might be nil.
        assert_node_exists(sibling,"__rb_rebalance_leaf");
        // Current node must be black by design of rb-tree.
        assert_node_color_black(node,"__rb_rebalance_leaf");
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
            __rb_color(parent) == __rb_black &&
            __rb_color(sibling) == __rb_black &&
            __rb_color(close_nephew) == __rb_black &&
            __rb_color(dist_nephew) == __rb_black
        )
        {
            __rb_set_color(sibling,__rb_red);
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
        if(__rb_color(sibling) == __rb_red)
        {
            #ifdef LOCKPICK_DEBUG
            // Check for red violation
            assert_node_color_black(parent,"__rb_rebalance_leaf");
            assert_node_color_black(close_nephew,"__rb_rebalance_leaf");
            assert_node_color_black(dist_nephew,"__rb_rebalance_leaf");
            // Nephews must be, otherwise black violation would take place
            assert_node_exists(close_nephew,"__rb_rebalance_leaf");
            assert_node_exists(dist_nephew,"__rb_rebalance_leaf");
            #endif // LOCKPICK_DEBUG

            if(root == parent)
                root = sibling;

            __rb_set_color(parent,__rb_red);
            __rb_set_color(sibling,__rb_black);
            if(rb_is_left_p(node,parent))
                __rb_rotate_left(parent);
            else
                __rb_rotate_right(parent);
            __rb_find_relatives(node,&parent,&sibling,&close_nephew,&dist_nephew);
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
            __rb_color(parent) == __rb_red &&
            __rb_color(close_nephew) == __rb_black &&
            __rb_color(dist_nephew) == __rb_black
        )
        {
            #ifdef LOCKPICK_DEBUG
            // Check for red violation
            assert_node_color_black(sibling,"__rb_rebalance_leaf");
            #endif // LOCKPICK_DEBUG

            __rb_set_color(sibling,__rb_red);
            __rb_set_color(parent,__rb_black);
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
            __rb_color(sibling) == __rb_black &&
            __rb_color(close_nephew) == __rb_red &&
            __rb_color(dist_nephew) == __rb_black
        )
        {
            __rb_set_color(sibling,__rb_red);
            __rb_set_color(close_nephew,__rb_black);
            if(rb_is_left_p(node,parent))
            {
                __rb_rotate_right(sibling);
                dist_nephew = sibling;
                sibling = close_nephew;
                close_nephew = close_nephew->left;
            }
            else
            {
                __rb_rotate_left(sibling);
                dist_nephew = sibling;
                sibling = close_nephew;
                close_nephew = close_nephew->right;
            }
        }

        #ifdef LOCKPICK_DEBUG
        // At this point distant sibling must be red by algorithm design
        assert_node_color_red(dist_nephew,"__rb_rebalance_leaf");
        // Check for red violation
        assert_node_color_black(sibling,"__rb_rebalance_leaf");
        // Sibling must be, otherwise black violation would take place. Though nephews might be nil.
        assert_node_exists(sibling,"__rb_rebalance_leaf");
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

        __rb_set_color(sibling,__rb_color(parent));
        __rb_set_color(parent,__rb_black);
        __rb_set_color(dist_nephew,__rb_black);
        if(rb_is_left_p(node,parent))
            __rb_rotate_left(parent);
        else
            __rb_rotate_right(parent);
        
        return root;

    } while(true);
}


/**
 * __rb_remove_rebalance_leaf - removes node without children and rebalances tree
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline struct rb_node *__rb_remove_rebalance_leaf(struct rb_node *root, struct rb_node *node)
{
    struct rb_node *parent = rb_parent(node);

    if(!parent)
        return NULL;

    if(__rb_color(node) == __rb_black)
        root = __rb_rebalance_leaf(root,node);

     __rb_rebind_child_from_parent(node,NULL,parent);

    return root;
}


/**
 * __rb_remove_node_one_child - removes node with one chilren and rebalances tree; this only child must be red
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * @non_nil_child: pointer on the only non-nil child of 'node'.
 * 
 * Returns root of the resulting tree.
 */
static inline struct rb_node *__rb_remove_node_one_child(struct rb_node *root, struct rb_node *node, struct rb_node *non_nil_child)
{
    #ifdef LOCKPICK_DEBUG
    // In case when node has exactly one non-nil child, this node must be black and child's color must be red. Otherwise it would be black violation.
    assert_node_color_black(node,"__rb_remove_rebalance_one_child");
    assert_node_color_red(non_nil_child,"__rb_remove_rebalance_one_child");

    assert(!node->left && node->right == non_nil_child || !node->right && node->left == non_nil_child && "__rb_remove_rebalance_one_child: Node must have only one child non-nil child that is equal to 'non_nil_child'.");
    #endif // LOCKPICK_DEBUG

    struct rb_node *parent = rb_parent(node);
    rb_set_parent(non_nil_child,parent);
    __rb_set_color(non_nil_child,__rb_black);
    if(parent)
    {
        __rb_rebind_child_from_parent(node,non_nil_child,parent);
        
        return root;
    }

    return non_nil_child;
}


/**
 * __rb_remove_rebalance - removes node with at most one child and rebalances tree
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
static inline struct rb_node *__rb_remove_rebalance(struct rb_node *root, struct rb_node *node)
{
    struct rb_node *non_nil_child = NULL;

    #ifdef LOCKPICK_DEBUG
    assert(!node->left || !node->right && "__rb_remove_rebalance_one_child: Node must have at least one nil child.");
    #endif // LOCKPICK_DEBUG

    if(node->left)
        non_nil_child = node->left;
    else if(node->right)
        non_nil_child = node->right;

    if(non_nil_child)
        return __rb_remove_node_one_child(root,node,non_nil_child);
    return __rb_remove_rebalance_leaf(root,node);
}


/**
 * rb_remove - removes node and rebalances tree
 * @root:   root of the tree; needed just for swift return in cases when original root is unaltered.
 * @node:	pointer to the current node.
 * 
 * Returns root of the resulting tree.
 */
struct rb_node *rb_remove(struct rb_node *root, struct rb_node *node)
{
    struct rb_node *to_rm = node;
    if(node->left != NULL && node->right != NULL)
    {
        struct rb_node *successor = __rb_get_left_most_child(node->right);
        struct rb_node *sc_parent = rb_parent(successor);
        struct rb_node *sc_left = successor->left;
        struct rb_node *sc_right = successor->right;
        struct rb_node *node_parent = rb_parent(node);
        enum __rb_colors sc_color = __rb_color(successor);

        if(!node_parent)
            root = successor;

        __rb_set_color(successor,__rb_color(node));
        __rb_set_color(node,sc_color);

        rb_set_parent(successor,node_parent);
        if(node_parent)
            __rb_rebind_child_from_parent(node,successor,node_parent);

        if(node->left)
            rb_set_parent(node->left,successor);
        successor->left = node->left;
        // node->right might point on successor
        if(successor == node->right)
        {
            successor->right = node;
            rb_set_parent(node,successor);
        }
        else
        {
            if(node->right)
                rb_set_parent(node->right,successor);
            successor->right = node->right;
            rb_set_parent(node,sc_parent);
            // successor is left child by design (if it is not a direct child of node)
            sc_parent->left = node;
        }

        if(sc_right)
            rb_set_parent(sc_right,node);
        node->right = sc_right;
        // successor->left is always null by design
        node->left = sc_left;
    }

    return __rb_remove_rebalance(root,node);
}


int __rb_count_black_nodes_one_path(const struct rb_node *root)
{
    int count = 0;
    while(root)
    {
        count += __rb_color(root) == __rb_black ? 1 : 0;
        root = root->left;
    }
    
    return count;
}


bool __rb_check_subtree_consistency(const struct rb_node *root, int blacks_required)
{
    if(!root)
        return true;

    struct rb_node *parent = rb_parent(root);
    if(parent && parent->left != root && parent->right != root)
        return false;

    if(__rb_color(root) == __rb_black)
        blacks_required -= 1;
    else
    {
        if(__rb_color(root->left) == __rb_red || __rb_color(root->right) == __rb_red)
            return false;
    }

    if(blacks_required < 0)
        return false;

    return __rb_check_subtree_consistency(root->left,blacks_required) && __rb_check_subtree_consistency(root->right,blacks_required);
}


bool rb_check_consistency(const struct rb_node *root)
{   
    int blacks_required = __rb_count_black_nodes_one_path(root);

    return __rb_check_subtree_consistency(root,blacks_required);
}