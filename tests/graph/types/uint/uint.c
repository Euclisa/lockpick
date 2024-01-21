#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <lockpick/affirmf.h>
#include <lockpick/graph/types/uint.h>
#include <lockpick/test.h>


#define HEXES_PER_WORD (sizeof(uint16_t)*2)
#define BITS_PER_WORD (sizeof(uint16_t)*8)
#define BITS_PER_HEX 4


static inline size_t pow2(size_t pow)
{
    static const size_t powers[] = {1, 2, 4, 8, 16};

    return powers[pow];
}


static inline void __rand_hex_str(char *dest, size_t width)
{
    static const char hexes[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    size_t hexes_num = (width-1)/BITS_PER_HEX + 1;
    size_t bits_remainder = width % BITS_PER_HEX;
    bits_remainder = bits_remainder == 0 ? BITS_PER_HEX : bits_remainder;
    dest[hexes_num] = '\0';

    dest[0] = hexes[rand()%pow2(bits_remainder)];
    for(size_t hex_i = 1; hex_i < hexes_num; ++hex_i)
        dest[hex_i] = hexes[rand()%sizeof(hexes)];
}


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


static inline size_t __count_line_in_file(FILE *fp)
{
    char ch;
    size_t count = 0;
    while((ch = fgetc(fp)) != EOF)
    {
        if (ch == '\n')
            ++count;
    }
    fseek(fp,0,SEEK_SET);

    return count;
}


void test_graph_uint_from_to_hex(size_t width)
{
    srand(0);
    const uint32_t tests_num = 1000;
    lpg_graph_t *graph = lpg_graph_create("test",1,1,(width+1)*tests_num);
    size_t hexes_num = (width-1)/__LPG_UINT_BITS_PER_HEX + 1;
    char *original_hex_str = (char*)malloc(hexes_num+1);
    char *converted_hex_str = (char*)malloc(hexes_num+1);
    lpg_uint_t *val = lpg_uint_create(width);
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        __rand_hex_str(original_hex_str,width);
        lpg_uint_from_hex(graph,original_hex_str,val);
        affirmf(lpg_uint_to_hex(val,converted_hex_str,hexes_num) > 0,
            "Failed to convert value to hex string");
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
    srand(0);
    const uint32_t tests_num = 1000;
    lpg_graph_t *graph = lpg_graph_create("test",1,1,width);
    size_t hexes_num = (width-1)/__LPG_UINT_BITS_PER_HEX + 1;
    char *original_hex_str = (char*)malloc(hexes_num+1);
    char *converted_hex_str = (char*)malloc(hexes_num+1);
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        lpg_uint_t *val = lpg_uint_create(width);
        __rand_hex_str(original_hex_str,hexes_num);
        lpg_uint_from_hex(graph,original_hex_str,val);
        affirmf(lpg_uint_to_hex(val,converted_hex_str,hexes_num) > 0,
            "Failed to convert value to hex string");
        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str),
            "Expected: %s, got: %s",original_hex_str,converted_hex_str);
    }
    free(original_hex_str);
    free(converted_hex_str);
}


void lp_test_graph_uint()
{
    for(size_t width = 1; width <= 64; ++width)
        LP_TEST_RUN(test_graph_uint_from_to_hex(width),0);
}