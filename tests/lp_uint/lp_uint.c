#include "lp_test.h"
#include "affirmf.h"
#include <lp_uint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define HEXES_PER_WORD (sizeof(uint16_t)*2)
#define BITS_PER_WORD (sizeof(uint16_t)*8)


char *__rand_hex_str(size_t hexes_num)
{
    static const char hexes[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    char *hex_str = (char*)malloc(hexes_num+1);
    hex_str[hexes_num] = '\0';

    for(size_t i = 0; i < hexes_num; ++i)
        hex_str[i] = hexes[rand()%sizeof(hexes)];
    
    return hex_str;
}


bool __hexcmp(const char *a, const char *b)
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

#define TEST_UINT_FROM_TO_HEX(N)                                                                                            \
void test_uint##N##_from_to_hex()                                                                                           \
{                                                                                                                           \
    srand(0);                                                                                                               \
    const uint32_t tests_num = 1000;                                                                                        \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                  \
    {                                                                                                                       \
        lp_uint(N) val;                                                                                                     \
        size_t hexes_num = N*__LP_UINT_HEXES_PER_WORD/__LP_UINT_BITS_PER_WORD;                                              \
        char *original_hex_str = __rand_hex_str(hexes_num);                                                                 \
        lp_uint_from_hex(val,original_hex_str);                                                                             \
        char *converted_hex_str = lp_uint_to_hex(val);                                                                      \
        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str),                                                        \
            "Expected: %s, got: %s\n",original_hex_str,converted_hex_str);                                                  \
        free(original_hex_str);                                                                                             \
        free(converted_hex_str);                                                                                            \
    }                                                                                                                       \
}                                                                                                                           \
void test_uint##N##_from_to_hex_overflow()                                                                                  \
{                                                                                                                           \
    srand(0);                                                                                                               \
    const uint32_t tests_num = 1000;                                                                                        \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                  \
    {                                                                                                                       \
        lp_uint(N) val;                                                                                                     \
        size_t hexes_num = N*__LP_UINT_HEXES_PER_WORD/__LP_UINT_BITS_PER_WORD;                                              \
        size_t hexes_num_overflowed = hexes_num + rand()%(N/__LP_UINT_BITS_PER_WORD*4);                                     \
        char *original_hex_str = __rand_hex_str(hexes_num_overflowed);                                                      \
        lp_uint_from_hex(val,original_hex_str);                                                                             \
        char *converted_hex_str = lp_uint_to_hex(val);                                                                      \
        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str+(hexes_num_overflowed-hexes_num)),                       \
            "Expected: %s, got: %s\n",original_hex_str+(hexes_num_overflowed-hexes_num),converted_hex_str);                 \
        free(original_hex_str);                                                                                             \
        free(converted_hex_str);                                                                                            \
    }                                                                                                                       \
}


TEST_UINT_FROM_TO_HEX(64)
TEST_UINT_FROM_TO_HEX(256)
TEST_UINT_FROM_TO_HEX(1024)


#define TEST_UINT_OPS(N_a, N_b, N_result)                                                                                   \
void test_uint_##N_a##_##N_b##_##N_result##_addition()                                                                      \
{                                                                                                                           \
    const char *fn_add = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_addition.txt";     \
    FILE *f_add = fopen(fn_add,"r");                                                                                        \
    affirmf(f_add,"Failed to open file '%s'",fn_add);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    char *hex_str_result_true = (char*)malloc(                                                                              \
        __LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+2);                             \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_add(a,b,res_obt),                                                                                   \
            "(%d,%d,%d,+) Failed to add pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                                \
        affirmf(fscanf(f_add,"%s\n",hex_str_result_true) != EOF,                                                            \
            "(%d,%d,%d,+) No result for pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                                \
        affirmf(lp_uint_from_hex(res_true,hex_str_result_true),                                                             \
            "(%d,%d,%d,+) Failed to parse res from hex string: %s",N_a,N_b,N_result,hex_str_result_true);                   \
        char *hex_str_result_obt = lp_uint_to_hex(res_obt);                                                                 \
        affirmf(hex_str_result_obt,                                                                                         \
            "(%d,%d,%d,+) Failed to convert res to hex string for pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);      \
        LP_TEST_ASSERT(lp_uint_eq(res_obt,res_true),"Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);      \
        free(hex_str_result_obt);                                                                                           \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_add);                                                                                                          \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    free(hex_str_result_true);                                                                                              \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_subtraction()                                                                   \
{                                                                                                                           \
    const char *fn_sub = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_subtraction.txt";  \
    FILE *f_sub = fopen(fn_sub,"r");                                                                                        \
    affirmf(f_sub,"Failed to open file '%s'",fn_sub);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    char *hex_str_result_true = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(2048)+1);                                \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_sub(a,b,res_obt),                                                                                   \
            "(%d,%d,%d,-) Failed to subtract pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                           \
        affirmf(fscanf(f_sub,"%s\n",hex_str_result_true) != EOF,                                                            \
            "(%d,%d,%d,-) No result for pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                                \
        affirmf(lp_uint_from_hex(res_true,hex_str_result_true),                                                             \
            "(%d,%d,%d,-) Failed to parse res from hex string: %s",N_a,N_b,N_result,hex_str_result_true);                   \
        char *hex_str_result_obt = lp_uint_to_hex(res_obt);                                                                 \
        affirmf(hex_str_result_obt,                                                                                         \
            "(%d,%d,%d,-) Failed to convert res to hex string for pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);      \
        LP_TEST_ASSERT(lp_uint_eq(res_obt,res_true),"Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);      \
        free(hex_str_result_obt);                                                                                           \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_sub);                                                                                                          \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    free(hex_str_result_true);                                                                                              \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_multiplication()                                                                \
{                                                                                                                           \
    const char *fn_mul =                                                                                                    \
        "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_multiplication.txt";                \
    FILE *f_mul = fopen(fn_mul,"r");                                                                                        \
    affirmf(f_mul,"Failed to open file '%s'",fn_mul);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    char *hex_str_result_true = (char*)malloc(                                                                              \
        __LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+2);                             \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_mul(a,b,res_obt),                                                                                   \
            "(%d,%d,%d,*) Failed to multiply pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                           \
        affirmf(fscanf(f_mul,"%s\n",hex_str_result_true) != EOF,                                                            \
            "(%d,%d,%d,*) No result for pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                                \
        affirmf(lp_uint_from_hex(res_true,hex_str_result_true),                                                             \
            "(%d,%d,%d,*) Failed to parse res from hex string: %s",N_a,N_b,N_result,hex_str_result_true);                   \
        char *hex_str_result_obt = lp_uint_to_hex(res_obt);                                                                 \
        affirmf(hex_str_result_obt,                                                                                         \
            "(%d,%d,%d,*) Failed to convert res to hex string for pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);      \
        LP_TEST_ASSERT(lp_uint_eq(res_obt,res_true),"Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);      \
        free(hex_str_result_obt);                                                                                           \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_mul);                                                                                                          \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    free(hex_str_result_true);                                                                                              \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_comparison()                                                                    \
{                                                                                                                           \
    const char *fn_comp =                                                                                                   \
        "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_comparison.txt";                    \
    FILE *f_comp = fopen(fn_comp,"r");                                                                                      \
    affirmf(f_comp,"Failed to open file '%s'",fn_comp);                                                                     \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    lp_uint_3way_t res_true,res_obt;                                                                                        \
    int32_t res_true_int;                                                                                                   \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        affirmf(fscanf(f_comp,"%d\n",&res_true_int) != EOF,                                                                 \
            "(%d,%d,%d,<=>) No result for pair: %s, %s",N_a,N_b,hex_str_a,hex_str_b);                                       \
        res_true = res_true_int;                                                                                            \
        res_obt = lp_uint_3way(a,b);                                                                                        \
        LP_TEST_ASSERT(res_true == res_obt,                                                                                 \
            "a: %s, b: %s. Expected comparison res: %d, got: %d",hex_str_a,hex_str_b,res_true_int,(int32_t)res_obt) ;       \
        LP_TEST_ASSERT(!(lp_uint_eq(a,b) && res_true != LP_UINT_EQUAL),                                                     \
            "a: %s, b: %s\nExpected %d, got: 'equal'",hex_str_a,hex_str_b,res_true_int);                                    \
        LP_TEST_ASSERT(!(lp_uint_ls(a,b) && res_true != LP_UINT_LESS),                                                      \
            "a: %s, b: %s\nExpected %d, got: 'less'",hex_str_a,hex_str_b,res_true_int);                                     \
        LP_TEST_ASSERT(!(lp_uint_gt(a,b) && res_true != LP_UINT_GREATER),                                                   \
            "a: %s, b: %s\nExpected %d, got: 'greater'",hex_str_a,hex_str_b,res_true_int);                                  \
        LP_TEST_ASSERT(!(lp_uint_geq(a,b) && res_true == LP_UINT_LESS),                                                     \
            "a: %s, b: %s\nExpected %d, got: 'greater-equal'",hex_str_a,hex_str_b,res_true_int);                            \
        LP_TEST_ASSERT(!(lp_uint_leq(a,b) && res_true == LP_UINT_GREATER),                                                  \
            "a: %s, b: %s\nExpected %d, got: 'less-equal'",hex_str_a,hex_str_b,res_true_int);                               \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_comp);                                                                                                         \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_ops()                                                                           \
{                                                                                                                           \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_addition());                                                         \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_subtraction());                                                      \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_multiplication());                                                   \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_comparison());                                                       \
}

TEST_UINT_OPS(64,64,256)
TEST_UINT_OPS(64,64,64)

TEST_UINT_OPS(64,128,256)
TEST_UINT_OPS(64,128,128)
TEST_UINT_OPS(64,128,64)

TEST_UINT_OPS(64,1024,256)
TEST_UINT_OPS(64,1024,64)
TEST_UINT_OPS(64,1024,1024)

TEST_UINT_OPS(256,256,1024)
TEST_UINT_OPS(256,256,512)
TEST_UINT_OPS(256,256,256)
TEST_UINT_OPS(256,256,64)

TEST_UINT_OPS(256,1024,1024)
TEST_UINT_OPS(256,1024,256)
TEST_UINT_OPS(256,1024,64)

TEST_UINT_OPS(1024,128,1024)
TEST_UINT_OPS(1024,128,128)
TEST_UINT_OPS(1024,128,64)

TEST_UINT_OPS(1024,256,1024)
TEST_UINT_OPS(1024,256,256)
TEST_UINT_OPS(1024,256,64)

TEST_UINT_OPS(1024,1024,2048)
TEST_UINT_OPS(1024,1024,1024)
TEST_UINT_OPS(1024,1024,64)


void test_lp_uint()
{
    LP_TEST_RUN(test_uint64_from_to_hex());
    LP_TEST_RUN(test_uint64_from_to_hex_overflow());

    LP_TEST_RUN(test_uint256_from_to_hex());
    LP_TEST_RUN(test_uint256_from_to_hex_overflow());

    LP_TEST_RUN(test_uint1024_from_to_hex());
    LP_TEST_RUN(test_uint1024_from_to_hex_overflow());


    LP_TEST_RUN(test_uint_64_64_256_ops());
    LP_TEST_RUN(test_uint_64_64_64_ops());

    LP_TEST_RUN(test_uint_64_128_256_ops());
    LP_TEST_RUN(test_uint_64_128_128_ops());
    LP_TEST_RUN(test_uint_64_128_64_ops());

    LP_TEST_RUN(test_uint_64_1024_1024_ops());
    LP_TEST_RUN(test_uint_64_1024_256_ops());
    LP_TEST_RUN(test_uint_64_1024_64_ops());

    LP_TEST_RUN(test_uint_256_256_1024_ops());
    LP_TEST_RUN(test_uint_256_256_512_ops());
    LP_TEST_RUN(test_uint_256_256_256_ops());
    LP_TEST_RUN(test_uint_256_256_64_ops());

    LP_TEST_RUN(test_uint_256_1024_1024_ops());
    LP_TEST_RUN(test_uint_256_1024_256_ops());
    LP_TEST_RUN(test_uint_256_1024_64_ops());

    LP_TEST_RUN(test_uint_1024_128_1024_ops());
    LP_TEST_RUN(test_uint_1024_128_128_ops());
    LP_TEST_RUN(test_uint_1024_128_64_ops());

    LP_TEST_RUN(test_uint_1024_256_1024_ops());
    LP_TEST_RUN(test_uint_1024_256_256_ops());
    LP_TEST_RUN(test_uint_1024_256_64_ops());

    LP_TEST_RUN(test_uint_1024_1024_2048_ops());
    LP_TEST_RUN(test_uint_1024_1024_1024_ops());
    LP_TEST_RUN(test_uint_1024_1024_64_ops());
}