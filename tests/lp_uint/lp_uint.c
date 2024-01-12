#include "lp_test.h"
#include "affirmf.h"
#include <lp_uint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>


#define HEXES_PER_WORD (sizeof(uint16_t)*2)
#define BITS_PER_WORD (sizeof(uint16_t)*8)


void __rand_hex_str(char *dest, size_t hexes_num)
{
    static const char hexes[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    dest[hexes_num] = '\0';
    for(size_t i = 0; i < hexes_num; ++i)
        dest[i] = hexes[rand()%sizeof(hexes)];
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
        const size_t hexes_num = N*__LP_UINT_HEXES_PER_WORD/__LP_UINT_BITS_PER_WORD;                                        \
        char original_hex_str[hexes_num+1];                                                                                 \
        char converted_hex_str[hexes_num+1];                                                                                \
        __rand_hex_str(original_hex_str,hexes_num);                                                                         \
        lp_uint_from_hex(val,original_hex_str);                                                                             \
        affirmf(lp_uint_to_hex(val,converted_hex_str,hexes_num) > 0,                                                        \
            "Failed to convert value to hex string");                                                                       \
        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str),                                                        \
            "Expected: %s, got: %s",original_hex_str,converted_hex_str);                                                    \
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
        char *original_hex_str = (char*)malloc(hexes_num_overflowed+1);                                                     \
        char *converted_hex_str = (char*)malloc(hexes_num_overflowed+1);                                                    \
        __rand_hex_str(original_hex_str,hexes_num_overflowed);                                                              \
        lp_uint_from_hex(val,original_hex_str);                                                                             \
        affirmf(lp_uint_to_hex(val,converted_hex_str,hexes_num_overflowed) > 0,                                             \
            "Failed to convert value to hex string");                                                                       \
        LP_TEST_ASSERT(__hexcmp(converted_hex_str,original_hex_str+(hexes_num_overflowed-hexes_num)),                       \
            "Expected: %s, got: %s",original_hex_str+(hexes_num_overflowed-hexes_num),converted_hex_str);                   \
        free(original_hex_str);                                                                                             \
        free(converted_hex_str);                                                                                            \
    }                                                                                                                       \
}


TEST_UINT_FROM_TO_HEX(64)
TEST_UINT_FROM_TO_HEX(256)
TEST_UINT_FROM_TO_HEX(1024)


#define CHUNK_READ_TRUE_RES_AND_TEST(file,hex_str_a,hex_str_b,hex_str_result_true)                                          \
    affirmf(fscanf(file,"%s\n",hex_str_result_true) != EOF,                                                                 \
        "No result for pair: %s, %s",hex_str_a,hex_str_b);                                                                  \
    affirmf(lp_uint_from_hex(res_true,hex_str_result_true),                                                                 \
        "Failed to parse res from hex string: %s",hex_str_result_true);                                                     \
    affirmf(lp_uint_to_hex(res_obt,hex_str_result_obt,hex_str_result_len) > 0,                                              \
        "Failed to convert value to hex string");                                                                           \
    LP_TEST_ASSERT(lp_uint_eq(res_obt,res_true),"Expected: %s, got: %s",hex_str_result_true,hex_str_result_obt);


#define CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                \
    const size_t hex_str_a_len = __LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a);                                                 \
    char hex_str_a[hex_str_a_len+1];                                                                                        \
    const size_t hex_str_b_len = __LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b);                                                 \
    char hex_str_b[hex_str_b_len+1];


#define CHUNK_INIT_HEX_STR_RESULTS(hex_str_result_len_v)                                                                    \
    const size_t hex_str_result_len = hex_str_result_len_v;                                                                 \
    char hex_str_result_true[hex_str_result_len+1];                                                                         \
    char hex_str_result_obt[hex_str_result_len+1];


#define TEST_UINT_OPS(N_a, N_b, N_result)                                                                                   \
void test_uint_##N_a##_##N_b##_##N_result##_addition()                                                                      \
{                                                                                                                           \
    const char *fn_add = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_addition.txt";     \
    FILE *f_add = fopen(fn_add,"r");                                                                                        \
    affirmf(f_add,"Failed to open file '%s'",fn_add);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    CHUNK_INIT_HEX_STR_RESULTS(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b))         \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_add(a,b,res_obt),                                                                                   \
            "Failed to add pair: %s, %s",hex_str_a,hex_str_b);                                                              \
        CHUNK_READ_TRUE_RES_AND_TEST(f_add,hex_str_a,hex_str_b,hex_str_result_true)                                         \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_add);                                                                                                          \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_subtraction()                                                                   \
{                                                                                                                           \
    const char *fn_sub = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_subtraction.txt";  \
    FILE *f_sub = fopen(fn_sub,"r");                                                                                        \
    affirmf(f_sub,"Failed to open file '%s'",fn_sub);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    CHUNK_INIT_HEX_STR_RESULTS(__LP_UINT_MAX_HEX_STR_REPRESENTATION(2048))                                                  \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_sub(a,b,res_obt),                                                                                   \
            "Failed to subtract pair: %s, %s",hex_str_a,hex_str_b);                                                         \
        CHUNK_READ_TRUE_RES_AND_TEST(f_sub,hex_str_a,hex_str_b,hex_str_result_true)                                         \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_sub);                                                                                                          \
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
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    CHUNK_INIT_HEX_STR_RESULTS(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b))         \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_mul(a,b,res_obt),                                                                                   \
            "Failed to multiply pair: %s, %s",N_a,N_b,N_result,hex_str_a,hex_str_b);                                        \
        CHUNK_READ_TRUE_RES_AND_TEST(f_mul,hex_str_a,hex_str_b,hex_str_result_true)                                         \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_mul);                                                                                                          \
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
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    lp_uint_3way_t res_true,res_obt;                                                                                        \
    int32_t res_true_int;                                                                                                   \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        affirmf(fscanf(f_comp,"%d\n",&res_true_int) != EOF,                                                                 \
            "No result for pair: %s, %s",hex_str_a,hex_str_b);                                                              \
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
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_and()                                                                           \
{                                                                                                                           \
    const char *fn_and =                                                                                                    \
        "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_and.txt";                           \
    FILE *f_and = fopen(fn_and,"r");                                                                                        \
    affirmf(f_and,"Failed to open file '%s'",fn_and);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    CHUNK_INIT_HEX_STR_RESULTS(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b))         \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_and(a,b,res_obt),                                                                                   \
            "Failed to bitwise 'and' pair: %s, %s",hex_str_a,hex_str_b);                                                    \
        CHUNK_READ_TRUE_RES_AND_TEST(f_and,hex_str_a,hex_str_b,hex_str_result_true)                                         \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_and);                                                                                                          \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_or()                                                                            \
{                                                                                                                           \
    const char *fn_or =                                                                                                     \
        "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_or.txt";                            \
    FILE *f_or = fopen(fn_or,"r");                                                                                          \
    affirmf(f_or,"Failed to open file '%s'",fn_or);                                                                         \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    CHUNK_INIT_HEX_STR_RESULTS(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b))         \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_or(a,b,res_obt),                                                                                    \
            "Failed to bitwise 'or' pair: %s, %s",hex_str_a,hex_str_b);                                                     \
        CHUNK_READ_TRUE_RES_AND_TEST(f_or,hex_str_a,hex_str_b,hex_str_result_true)                                          \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_or);                                                                                                           \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_xor()                                                                           \
{                                                                                                                           \
    const char *fn_xor =                                                                                                    \
        "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_xor.txt";                           \
    FILE *f_xor = fopen(fn_xor,"r");                                                                                        \
    affirmf(f_xor,"Failed to open file '%s'",fn_xor);                                                                       \
    const char *fn_samp = "/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt";             \
    FILE *f_samp = fopen(fn_samp,"r");                                                                                      \
    affirmf(f_samp,"Failed to open file '%s'",fn_samp);                                                                     \
    CHUNK_INIT_HEX_STR_OPERANDS(N_a,N_b)                                                                                    \
    CHUNK_INIT_HEX_STR_RESULTS(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b))         \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        lp_uint(N_a) a;                                                                                                     \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        lp_uint(N_b) b;                                                                                                     \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        lp_uint(N_result) res_obt, res_true;                                                                                \
        affirmf(lp_uint_xor(a,b,res_obt),                                                                                   \
            "Failed to bitwise 'xor' pair: %s, %s",hex_str_a,hex_str_b);                                                    \
        CHUNK_READ_TRUE_RES_AND_TEST(f_xor,hex_str_a,hex_str_b,hex_str_result_true)                                         \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_xor);                                                                                                          \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_ops_arithmetic()                                                                \
{                                                                                                                           \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_addition());                                                         \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_subtraction());                                                      \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_multiplication());                                                   \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_ops_bitwise()                                                                   \
{                                                                                                                           \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_and());                                                              \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_or());                                                               \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_xor());                                                              \
}                                                                                                                           \
void test_uint_##N_a##_##N_b##_##N_result##_ops()                                                                           \
{                                                                                                                           \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_ops_arithmetic());                                                   \
    LP_TEST_RUN(test_uint_##N_a##_##N_b##_##N_result##_ops_bitwise());                                                      \
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