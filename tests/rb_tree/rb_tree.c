#include <time.h>
#include <stdlib.h>
#include <lockpick/rb_tree.h>
#include <lockpick/container_of.h>
#include <lockpick/test.h>
#include <lockpick/slab/slab.h>


#define RBTEST_MAX_ELEMENTS_IN_TREE 1000
#define RBTEST_TESTS_NUM 100

struct int_rb_node
{
    int value;
    lp_rb_node_t __rb_node; 
};


void swap(int *first, int *second)
{
    int c = *second;
    *second = *first;
    *first = c;
}


void shuffle(int *arr, int length)
{
    for(int i = 0; i < length * 2; ++i)
    {
        int first_i = rand() % length;
        int second_i = rand() % length;
        swap(arr+first_i,arr+second_i);
    }
}


lp_rb_node_t *int_rb_insert(lp_rb_node_t *root, struct int_rb_node *node)
{
    if(root)
    {
        struct int_rb_node *current = container_of(root,struct int_rb_node,__rb_node);
        while(1)
        {
            lp_rb_node_t **next_child;
            next_child = current->value < node->value ? &current->__rb_node.right : &current->__rb_node.left;
            if(!*next_child)
            {
                *next_child = &node->__rb_node;
                lp_rb_set_parent(&node->__rb_node,&current->__rb_node);
                break;
            }
            current = container_of(*next_child,struct int_rb_node,__rb_node);
        }
    }
    else
        lp_rb_set_parent(&node->__rb_node,NULL);
    node->__rb_node.left = NULL;
    node->__rb_node.right = NULL;

    return lp_rb_insert_rebalance(root,&node->__rb_node);
}


struct int_rb_node *int_rb_find(lp_rb_node_t *root, int value)
{
    if(root)
    {
        struct int_rb_node *current = container_of(root,struct int_rb_node,__rb_node);
        while(current)
        {
            if(current->value == value)
                return current;

            lp_rb_node_t *next_child = current->value < value ? current->__rb_node.right : current->__rb_node.left;
            current = container_of(next_child,struct int_rb_node,__rb_node);
        }
    }

    return NULL;
}


void test_rb_tree_monotonic_insert_random_remove(size_t tests_num, size_t elements_in_tree)
{
    lp_slab_t *rb_slab = lp_slab_create(RBTEST_MAX_ELEMENTS_IN_TREE,sizeof(struct int_rb_node));
    for(int test_i = 0; test_i < tests_num; ++test_i)
    {
        lp_rb_node_t *root = NULL;

        int values[RBTEST_MAX_ELEMENTS_IN_TREE];

        for(int i = 0; i < elements_in_tree; ++i)
        {
            struct int_rb_node *node = lp_slab_alloc(rb_slab);
            node->value = i;
            values[i] = node->value;
            root = int_rb_insert(root,node);
            LP_TEST_ASSERT(lp_rb_check_consistency(root), "Red-black tree is not consistent on insertion.");
        }

        shuffle(values, elements_in_tree);

        for(int i = 0; i < elements_in_tree; ++i)
        {
            struct int_rb_node *node = int_rb_find(root,values[i]);
            root = lp_rb_remove(root,&node->__rb_node);
            LP_TEST_ASSERT(lp_rb_check_consistency(root), "Red-black tree is not consistent on deletion.");
            lp_slab_free(rb_slab,node);
        }
    }
    lp_test_cleanup:
    lp_slab_release(rb_slab);
}


void test_rb_tree_random_insert_random_remove(size_t tests_num, size_t elements_in_tree)
{
    lp_slab_t *rb_slab = lp_slab_create(RBTEST_MAX_ELEMENTS_IN_TREE,sizeof(struct int_rb_node));
    for(int test_i = 0; test_i < tests_num; ++test_i)
    {
        lp_rb_node_t *root = NULL;

        int values[RBTEST_MAX_ELEMENTS_IN_TREE];

        for(int i = 0; i < elements_in_tree; ++i)
        {
            struct int_rb_node *node = lp_slab_alloc(rb_slab);
            node->value = i;
            values[i] = node->value;
            root = int_rb_insert(root,node);
            LP_TEST_ASSERT(lp_rb_check_consistency(root), "Red-black tree is not consistent on insertion.");
        }

        shuffle(values, elements_in_tree);

        for(int i = 0; i < elements_in_tree; ++i)
        {
            struct int_rb_node *node = int_rb_find(root,values[i]);
            root = lp_rb_remove(root,&node->__rb_node);
            LP_TEST_ASSERT(lp_rb_check_consistency(root), "Red-black tree is not consistent on deletion.");
            lp_slab_free(rb_slab,node);
        }
    }
    lp_test_cleanup:
    lp_slab_release(rb_slab);
}


void test_rb_tree_random_insert_root_remove(size_t tests_num, size_t elements_in_tree)
{
    lp_slab_t *rb_slab = lp_slab_create(RBTEST_MAX_ELEMENTS_IN_TREE,sizeof(struct int_rb_node));
    for(int test_i = 0; test_i < tests_num; ++test_i)
    {
        lp_rb_node_t *root = NULL;

        for(int i = 0; i < elements_in_tree; ++i)
        {
            struct int_rb_node *node = lp_slab_alloc(rb_slab);
            node->value = i;
            root = int_rb_insert(root,node);
            LP_TEST_ASSERT(lp_rb_check_consistency(root), "Red-black tree is not consistent on insertion.");
        }

        for(int i = 0; i < elements_in_tree; ++i)
        {
            struct int_rb_node *node = container_of(root,struct int_rb_node,__rb_node);
            root = lp_rb_remove(root,&node->__rb_node);
            LP_TEST_ASSERT(lp_rb_check_consistency(root), "Red-black tree is not consistent on deletion.");
            lp_slab_free(rb_slab,node);
        }
    }
    lp_test_cleanup:
    lp_slab_release(rb_slab);
}


void lp_test_rb_tree()
{
    srand(0);
    LP_TEST_RUN(test_rb_tree_monotonic_insert_random_remove(1000,50));
    LP_TEST_RUN(test_rb_tree_monotonic_insert_random_remove(100,500));
    //LP_TEST_RUN(test_rb_tree_monotonic_insert_random_remove(30,1000));
    LP_TEST_RUN(test_rb_tree_random_insert_random_remove(1000,50));
    LP_TEST_RUN(test_rb_tree_random_insert_random_remove(100,500));
    //LP_TEST_RUN(test_rb_tree_random_insert_random_remove(30,1000));
    LP_TEST_RUN(test_rb_tree_random_insert_root_remove(1000,50));
    LP_TEST_RUN(test_rb_tree_random_insert_root_remove(100,500));
    //LP_TEST_RUN(test_rb_tree_random_insert_root_remove(30,1000));
}