#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <lockpick/affirmf.h>
#include <lockpick/graph/types/uint.h>
#include <lockpick/uint.h>
#include <lockpick/test.h>

#define __LPG_TEST_UINT_MAX_GRAPH_NODES 100000

static inline bool __hexcmp(const char *a, const char *b)
{
    size_t leading_zeros_offset_a = 0;
    size_t len_a = strlen(a);
    size_t leading_zeros_offset_b = 0;
    size_t len_b = strlen(b);
    if(len_a == 0 || len_b == 0)
        return false;

    while(a[leading_zeros_offset_a] == '0' && leading_zeros_offset_a < len_a-1)
        ++leading_zeros_offset_a;

    while(b[leading_zeros_offset_b] == '0' && leading_zeros_offset_b < len_b-1)
        ++leading_zeros_offset_b;
    
    if((len_a-leading_zeros_offset_a) != (len_b-leading_zeros_offset_b))
        return false;
    
    while(leading_zeros_offset_a != len_a)
    {
        if(a[leading_zeros_offset_a++] != b[leading_zeros_offset_b++])
            return false;
    }

    return true;
}


typedef lp_uint_t(1024) cases_uint_t;
const size_t MAX_HEXES_NUM = sizeof(cases_uint_t)*2;
const size_t MAX_WIDTH = sizeof(cases_uint_t)*8;


void test_graph_uint_from_to_hex(size_t width)
{
    const uint32_t tests_num = 1000;

    lpg_graph_t *graph = lpg_graph_create("test",width,width,(width+1)*tests_num);
    for(size_t node_i = 0; node_i < graph->inputs_size; ++node_i)
    {
        __lpg_node_set_computed(graph->inputs[node_i],true);
        graph->outputs[node_i] = graph->inputs[node_i];
    }

    char *original_hex_str = (char*)malloc(MAX_HEXES_NUM+1);
    char *converted_hex_str = (char*)malloc(MAX_HEXES_NUM+1);
    lpg_uint_t *val = lpg_uint_allocate_as_buffer_view(graph,graph->inputs,width);

    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        cases_uint_t val_prop;
        lp_uint_rand(val_prop,width);
        lp_uint_to_hex(val_prop,original_hex_str,MAX_HEXES_NUM);

        lpg_uint_update_from_uint(val,val_prop);
        lpg_uint_to_hex(val,converted_hex_str,MAX_HEXES_NUM);

        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str),
            "Expected: %s, got: %s",original_hex_str,converted_hex_str);
    }
    lpg_graph_release(graph);
    lpg_uint_release(val);
    free(original_hex_str);
    free(converted_hex_str);
}


#define TEST_GRAPH_UINT_OP_INIT_CHUNK(res_width)                                                                                \
    lpg_graph_t *graph = lpg_graph_create("test",a_width+b_width,res_width,__LPG_TEST_UINT_MAX_GRAPH_NODES);                    \
    for(size_t node_i = 0; node_i < graph->inputs_size; ++node_i)                                                               \
        __lpg_node_set_computed(graph->inputs[node_i],true);                                                                    \
    char *original_hex_str = (char*)malloc(MAX_HEXES_NUM+1);                                                                    \
    char *converted_hex_str = (char*)malloc(MAX_HEXES_NUM+1);                                                                   \
    lpg_uint_t *graph_a = lpg_uint_allocate_as_buffer_view(graph,graph->inputs,a_width);                                               \
    lpg_uint_t *graph_b = lpg_uint_allocate_as_buffer_view(graph,graph->inputs+a_width,b_width);                                       \
    lpg_uint_t *graph_res_obt = lpg_uint_allocate_as_buffer_view(graph,graph->outputs,res_width);

#define TEST_GRAPH_UINT_OP_CLEAN_UP_CHUNK                                                                                       \
    lpg_graph_release(graph);                                                                                                   \
    lpg_uint_release(graph_a);                                                                                                  \
    lpg_uint_release(graph_b);                                                                                                  \
    lpg_uint_release(graph_res_obt);                                                                                            \
    free(original_hex_str);                                                                                                     \
    free(converted_hex_str);


#define __TEST_GRAPH_UINT_OP_GEN_AND_TEST(op_type,postfix)                                                                      \
static inline void __test_graph_uint_gen_and_test_##op_type##_##postfix(                                                        \
        lpg_graph_t *graph,                                                                                                     \
        lpg_uint_t *graph_a,                                                                                                    \
        lpg_uint_t *graph_b,                                                                                                    \
        lpg_uint_t *graph_res_obt,                                                                                              \
        char *original_hex_str,                                                                                                 \
        char *converted_hex_str)                                                                                                \
{                                                                                                                               \
    char *hex_str_a = (char*)malloc(MAX_HEXES_NUM+1);                                                                           \
    char *hex_str_b = (char*)malloc(MAX_HEXES_NUM+1);                                                                           \
    cases_uint_t a_prop,b_prop,res_true_prop,res_obt_prop;                                                                      \
    /* Generate samples */                                                                                                      \
    lp_uint_rand(a_prop,graph_a->width);                                                                                        \
    lp_uint_rand(b_prop,graph_b->width);                                                                                        \
    lp_uint_to_hex(a_prop,hex_str_a,MAX_HEXES_NUM);                                                                             \
    lp_uint_to_hex(b_prop,hex_str_b,MAX_HEXES_NUM);                                                                             \
    lp_uint_##op_type(a_prop,b_prop,res_true_prop);                                                                             \
    /* Build mask for truncating result to appropriate width */                                                                 \
    cases_uint_t __res_mask,__one;                                                                                              \
    lp_uint_from_hex(__res_mask,"1");                                                                                           \
    lp_uint_from_hex(__one,"1");                                                                                                \
    lp_uint_lshift_ip(__res_mask,graph_res_obt->width);                                                                         \
    lp_uint_sub_ip(__res_mask,__one);                                                                                           \
    /* Mask result and convert to hex string for printing error info */                                                         \
    lp_uint_and_ip(res_true_prop,__res_mask);                                                                                   \
    lp_uint_to_hex(res_true_prop,original_hex_str,MAX_HEXES_NUM);                                                               \
    /* Update graph uint operand values from previously generated ones */                                                       \
    lpg_uint_update_from_uint(graph_a,a_prop);                                                                                  \
    lpg_uint_update_from_uint(graph_b,b_prop);                                                                                  \
    /* Compute new graph value and convert it to uint */                                                                        \
    lpg_graph_reset(graph);                                                                                                     \
    lpg_graph_compute(graph);                                                                                                   \
    lpg_uint_to_hex(graph_res_obt,converted_hex_str,MAX_HEXES_NUM);                                                             \
    lp_uint_from_hex(res_obt_prop,converted_hex_str);                                                                           \
    LP_TEST_ASSERT(lp_uint_eq(res_true_prop,res_obt_prop),                                                                      \
            "a: %s; b: %s; Expected: %s, got: %s",hex_str_a,hex_str_b,original_hex_str,converted_hex_str);                      \
    free(hex_str_a);                                                                                                            \
    free(hex_str_b);                                                                                                            \
}


#define TEST_GRAPH_UINT_OP(op_type, width_high, cases_num)                                                                      \
__TEST_GRAPH_UINT_OP_GEN_AND_TEST(op_type,plain)                                                                                \
void __test_graph_uint_##op_type(size_t a_width, size_t b_width, size_t res_width)                                              \
{                                                                                                                               \
    const uint32_t tests_num = cases_num;                                                                                       \
    TEST_GRAPH_UINT_OP_INIT_CHUNK(res_width)                                                                                    \
    lpg_uint_##op_type(graph_a,graph_b,graph_res_obt);                                                                          \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                      \
    {                                                                                                                           \
        LP_TEST_STEP_INTO(                                                                                                      \
            __test_graph_uint_gen_and_test_##op_type##_plain(                                                                   \
                graph,graph_a,graph_b,graph_res_obt,                                                                            \
                original_hex_str,converted_hex_str)                                                                             \
        );                                                                                                                      \
    }                                                                                                                           \
    TEST_GRAPH_UINT_OP_CLEAN_UP_CHUNK                                                                                           \
}                                                                                                                               \
void test_graph_uint_##op_type()                                                                                                \
{                                                                                                                               \
    for(size_t width = 1; width <= width_high; ++width)                                                                         \
    {                                                                                                                           \
        LP_TEST_STEP_INTO(__test_graph_uint_##op_type(width,width,width));                                                      \
        LP_TEST_STEP_INTO(__test_graph_uint_##op_type(width,width,width/2+1));                                                  \
        LP_TEST_STEP_INTO(__test_graph_uint_##op_type(width,width/2+1,width));                                                  \
        LP_TEST_STEP_INTO(__test_graph_uint_##op_type(width/2+1,width,width));                                                  \
        LP_TEST_STEP_INTO(__test_graph_uint_##op_type(width/2+1,width,width*2));                                                \
        LP_TEST_STEP_INTO(__test_graph_uint_##op_type(width,width/2+1,width/2+1));                                              \
    }                                                                                                                           \
}


#define TEST_GRAPH_UINT_OP_INPLACE(op_type, width_high, cases_num)                                                              \
__TEST_GRAPH_UINT_OP_GEN_AND_TEST(op_type,inplace)                                                                              \
void __test_graph_uint_##op_type##_inplace(size_t a_width, size_t b_width)                                                      \
{                                                                                                                               \
    const uint32_t tests_num = cases_num;                                                                                       \
    TEST_GRAPH_UINT_OP_INIT_CHUNK(a_width)                                                                                      \
    lpg_uint_copy(graph_res_obt,graph_a);                                                                                       \
    lpg_uint_##op_type##_ip(graph_res_obt,graph_b);                                                                             \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                      \
    {                                                                                                                           \
        LP_TEST_STEP_INTO(                                                                                                      \
            __test_graph_uint_gen_and_test_##op_type##_inplace(                                                                 \
                graph,graph_a,graph_b,graph_res_obt,                                                                            \
                original_hex_str,converted_hex_str)                                                                             \
        );                                                                                                                      \
    }                                                                                                                           \
    TEST_GRAPH_UINT_OP_CLEAN_UP_CHUNK                                                                                           \
}                                                                                                                               \
void test_graph_uint_##op_type##_inplace()                                                                                      \
{                                                                                                                               \
    for(size_t width = 1; width <= width_high; ++width)                                                                         \
    {                                                                                                                           \
        LP_TEST_RUN(__test_graph_uint_##op_type##_inplace(width,width),0);                                                      \
        LP_TEST_RUN(__test_graph_uint_##op_type##_inplace(width,width/2+1),0);                                                  \
        LP_TEST_RUN(__test_graph_uint_##op_type##_inplace(width/2+1,width),0);                                                  \
        LP_TEST_RUN(__test_graph_uint_##op_type##_inplace(width*2+1,width),0);                                                  \
        LP_TEST_RUN(__test_graph_uint_##op_type##_inplace(width,width*2),0);                                                    \
    }                                                                                                                           \
}

TEST_GRAPH_UINT_OP(add,64,10)
TEST_GRAPH_UINT_OP_INPLACE(add,64,10)
TEST_GRAPH_UINT_OP(sub,64,10)
TEST_GRAPH_UINT_OP(mul,32,3)

TEST_GRAPH_UINT_OP(and,64,10)
TEST_GRAPH_UINT_OP_INPLACE(and,64,10)
TEST_GRAPH_UINT_OP(or,64,10)
TEST_GRAPH_UINT_OP_INPLACE(or,64,10)
TEST_GRAPH_UINT_OP(xor,64,10)
TEST_GRAPH_UINT_OP_INPLACE(xor,64,10)


void test_graph_uint_hex_str()
{
    for(size_t width = 1; width <= 64; ++width)
    {
        LP_TEST_RUN(test_graph_uint_from_to_hex(width));
    }
}


void lp_test_graph_uint()
{
    srand(0);

    LP_TEST_RUN(test_graph_uint_hex_str(),1);
    LP_TEST_RUN(test_graph_uint_add());
    LP_TEST_RUN(test_graph_uint_add_inplace());
    LP_TEST_RUN(test_graph_uint_sub());
    LP_TEST_RUN(test_graph_uint_mul());

    LP_TEST_RUN(test_graph_uint_and());
    LP_TEST_RUN(test_graph_uint_and_inplace());
    LP_TEST_RUN(test_graph_uint_or());
    LP_TEST_RUN(test_graph_uint_or_inplace());
    LP_TEST_RUN(test_graph_uint_xor());
    LP_TEST_RUN(test_graph_uint_xor_inplace());
}