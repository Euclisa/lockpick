#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <lockpick/affirmf.h>
#include <lockpick/graph/types/uint.h>
#include <lockpick/uint.h>
#include <lockpick/test.h>


#define HEXES_PER_WORD (sizeof(uint16_t)*2)
#define BITS_PER_WORD (sizeof(uint16_t)*8)
#define BITS_PER_HEX 4

#define __LPG_TEST_UINT_MAX_WIDTH 256


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
    lpg_uint_t *val = lpg_uint_create(graph,graph->inputs,width);

    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        cases_uint_t val_prop;
        lp_uint_rand(val_prop,width);
        lp_uint_to_hex(val_prop,original_hex_str,MAX_HEXES_NUM);

        lpg_uint_from_uint(val,val_prop);
        lpg_uint_to_hex(val,converted_hex_str,MAX_HEXES_NUM);

        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str),
            "Expected: %s, got: %s",original_hex_str,converted_hex_str);
    }
    lpg_graph_release(graph);
    lpg_uint_release(val);
    free(original_hex_str);
    free(converted_hex_str);
}


void test_graph_uint_from_to_hex_overflow(size_t width)
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
    lpg_uint_t *val = lpg_uint_create(graph,graph->inputs,width);

    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        cases_uint_t val_prop;
        lp_uint_rand(val_prop,width);
        lp_uint_to_hex(val_prop,original_hex_str,MAX_HEXES_NUM);

        lpg_uint_from_uint(val,val_prop);
        lpg_uint_to_hex(val,converted_hex_str,MAX_HEXES_NUM);

        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str),
            "Expected: %s, got: %s",original_hex_str,converted_hex_str);
    }
    lpg_graph_release(graph);
    lpg_uint_release(val);
    free(original_hex_str);
    free(converted_hex_str);
}


void test_graph_uint_addition(size_t a_width, size_t b_width, size_t res_width)
{
    const uint32_t tests_num = 10;

    lpg_graph_t *graph = lpg_graph_create("test",a_width+b_width,res_width,(a_width+b_width+res_width+1)*tests_num);

    for(size_t node_i = 0; node_i < graph->inputs_size; ++node_i)
        __lpg_node_set_computed(graph->inputs[node_i],true);

    char *original_hex_str = (char*)malloc(MAX_HEXES_NUM+1);
    char *converted_hex_str = (char*)malloc(MAX_HEXES_NUM+1);

    lpg_uint_t *graph_a = lpg_uint_create(graph,graph->inputs,a_width);
    lpg_uint_t *graph_b = lpg_uint_create(graph,graph->inputs+a_width,b_width);
    lpg_uint_t *graph_res_obt = lpg_uint_create(graph,graph->outputs,res_width);

    lpg_uint_add(graph,graph_a,graph_b,graph_res_obt);

    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        cases_uint_t a_prop,b_prop,res_true_prop,res_obt_prop;
        lp_uint_rand(a_prop,a_width);
        lp_uint_rand(b_prop,b_width);
        lp_uint_add(a_prop,b_prop,res_true_prop);
        cases_uint_t __res_mask,__one;
        lp_uint_from_hex(__res_mask,"1");
        lp_uint_from_hex(__one,"1");
        lp_uint_lshift_ip(__res_mask,res_width);
        lp_uint_sub_ip(__res_mask,__one);
        lp_uint_and_ip(res_true_prop,__res_mask);
        lp_uint_to_hex(res_true_prop,original_hex_str,MAX_HEXES_NUM);
        
        lpg_uint_from_uint(graph_a,a_prop);
        lpg_uint_from_uint(graph_b,b_prop);

        lpg_graph_compute(graph);

        lpg_uint_to_hex(graph_res_obt,converted_hex_str,MAX_HEXES_NUM);
        lp_uint_from_hex(res_obt_prop,converted_hex_str);

        LP_TEST_ASSERT(lp_uint_eq(res_true_prop,res_obt_prop),
                "Expected: %s, got: %s",original_hex_str,converted_hex_str);

        lpg_graph_reset(graph);
    }
    lpg_graph_release(graph);
    free(original_hex_str);
    free(converted_hex_str);
}

void lp_test_graph_uint()
{
    srand(0);

    for(size_t width = 1; width <= 64; ++width)
    {
        LP_TEST_RUN(test_graph_uint_from_to_hex(width));
        LP_TEST_RUN(test_graph_uint_from_to_hex_overflow(width));
    }

    for(size_t width = 1; width <= 64; ++width)
    {
        LP_TEST_RUN(test_graph_uint_addition(width,width,width));
        LP_TEST_RUN(test_graph_uint_addition(width,width,width/2+1));
        LP_TEST_RUN(test_graph_uint_addition(width,width/2+1,width));
        LP_TEST_RUN(test_graph_uint_addition(width/2+1,width,width));
        LP_TEST_RUN(test_graph_uint_addition(width/2+1,width,width*2));
        LP_TEST_RUN(test_graph_uint_addition(width,width/2+1,width/2+1));
    }
}