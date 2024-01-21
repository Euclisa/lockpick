#include <lockpick/graph/graph.h>
#include <lockpick/affirmf.h>
#include <lockpick/list.h>
#include <lockpick/container_of.h>


typedef struct __lp_node_stack
{
    lp_list_t __list;
    lpg_node_t *node;
} __lp_node_stack_t;


static inline bool __lp_node_stack_push(__lp_node_stack_t **stack, lpg_node_t *node)
{
    __lp_node_stack_t *entry = (__lp_node_stack_t*)malloc(sizeof(__lp_node_stack_t));
    entry->node = node;
    lp_list_t *old_head = *stack ? &(*stack)->__list : NULL;
    affirmf(lp_list_push_front(&old_head,&entry->__list),"Failed to push to node computing stack");
    
    *stack = container_of(old_head,__lp_node_stack_t,__list);

    return true;
}

static inline void __lp_node_stack_pop(__lp_node_stack_t **stack)
{
    lp_list_t *old_head = &(*stack)->__list;
    __lp_node_stack_t *top = (*stack);
    affirmf(lp_list_remove(&old_head,&(*stack)->__list),"Failed to remove top from node computing stack");

    *stack = container_of(old_head,__lp_node_stack_t,__list);
    free(top);
}


bool lpg_node_compute(lpg_node_t *node)
{
    __lp_node_stack_t *stack = NULL;
    __lp_node_stack_push(&stack,node);
    
    while(stack)
    {
        lpg_node_t *curr_node = stack->node;
        __lp_node_stack_pop(&stack);

        uint16_t curr_node_operands_num = lpg_node_type_operands_num[curr_node->type];
        lpg_node_t **curr_node_parents = lpg_node_parents(curr_node);
        bool curr_node_ready = true;
        for(uint16_t parent_i = 0; parent_i < curr_node_operands_num; ++parent_i)
        {
            lpg_node_t *parent = curr_node_parents[parent_i];
            if(!lpg_node_computed(parent))
            {
                curr_node_ready = false;
                __lp_node_stack_push(&stack,parent);
            }
        }
        if(!curr_node_ready)
        {
            __lp_node_stack_push(&stack,curr_node);
            continue;
        }

        switch(curr_node->type)
        {
            case LPG_NODE_TYPE_AND:
            {
                bool a_val = lpg_node_value(curr_node_parents[0]);
                bool b_val = lpg_node_value(curr_node_parents[1]);
                return a_val && b_val;
            }
            case LPG_NODE_TYPE_OR:
            {
                bool a_val = lpg_node_value(curr_node_parents[0]);
                bool b_val = lpg_node_value(curr_node_parents[1]);
                return a_val || b_val;
            }
            case LPG_NODE_TYPE_NOT:
            {
                bool a_val = lpg_node_value(curr_node_parents[0]);
                return !a_val;
            }
            case LPG_NODE_TYPE_XOR:
            {
                bool a_val = lpg_node_value(curr_node_parents[0]);
                bool b_val = lpg_node_value(curr_node_parents[1]);
                return a_val ^ b_val;
            }
            case LPG_NODE_TYPE_TRUE:
                return true;

            case LPG_NODE_TYPE_FALSE:
                return false;

            case LPG_NODE_TYPE_VAR:
                affirmf(lpg_node_computed(curr_node),
                    "Failed to compute node:"
                    "nodes of type 'LPG_NODE_TYPE_VAR' must have 'computed' falg set in order to fetch their value.");

                bool curr_val = lpg_node_value(curr_node);
                return curr_val;

            default:
                errorf("Invalid operation type: %d",node->type);
        }
    }

    return false;
}