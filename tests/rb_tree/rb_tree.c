#include <rb_tree.h>
#include "container_of.h"
#include <time.h>
#include <stdlib.h>


#define RBTEST_ELEMENTS_IN_TREE 1000
#define RBTEST_TESTS_NUM 100

struct int_rb_node
{
    int value;
    struct rb_node __rb_node; 
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


struct rb_node *int_rb_insert(struct rb_node *root, struct int_rb_node *node)
{
    if(root)
    {
        struct int_rb_node *current = container_of(root,struct int_rb_node,__rb_node);
        while(1)
        {
            struct rb_node **next_child;
            next_child = current->value < node->value ? &current->__rb_node.right : &current->__rb_node.left;
            if(!*next_child)
            {
                *next_child = &node->__rb_node;
                rb_set_parent(&node->__rb_node,&current->__rb_node);
                break;
            }
            current = container_of(*next_child,struct int_rb_node,__rb_node);
        }
    }

    return rb_insert_rebalance(root,&node->__rb_node);
}


struct int_rb_node *int_rb_find(struct rb_node *root, int value)
{
    if(root)
    {
        struct int_rb_node *current = container_of(root,struct int_rb_node,__rb_node);
        while(current)
        {
            if(current->value == value)
                return current;

            struct rb_node *next_child = current->value < value ? current->__rb_node.right : current->__rb_node.left;
            current = container_of(next_child,struct int_rb_node,__rb_node);
        }
    }

    return NULL;
}


void test_rb_tree()
{
    srand(0);
    for(int test_i = 0; test_i < RBTEST_TESTS_NUM; ++test_i)
    {
        struct rb_node *root = NULL;

        int values[RBTEST_ELEMENTS_IN_TREE];

        for(int i = 0; i < RBTEST_ELEMENTS_IN_TREE; ++i)
        {
            struct int_rb_node *node = (struct int_rb_node*)malloc(sizeof(struct int_rb_node));
            node->value = rand();
            values[i] = node->value;
            root = int_rb_insert(root,node);
            assert(rb_check_consistency(root) && "Test failed. Red-black tree is not consistent.");
        }

        shuffle(values, RBTEST_ELEMENTS_IN_TREE);

        for(int i = 0; i < RBTEST_ELEMENTS_IN_TREE; ++i)
        {
            struct int_rb_node *node = int_rb_find(root,values[i]);
            root = rb_remove(root,&node->__rb_node);
            assert(rb_check_consistency(root) && "Test failed. Red-black tree is not consistent.");
        }
    }
}