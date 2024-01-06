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
bool test_uint##N##_from_to_hex()                                                                                           \
{                                                                                                                           \
    srand(0);                                                                                                               \
    const uint32_t tests_num = 1000;                                                                                        \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                  \
    {                                                                                                                       \
        uint(N) val;                                                                                                        \
        size_t hexes_num = N*__LP_UINT_HEXES_PER_WORD/__LP_UINT_BITS_PER_WORD;                                              \
        char *original_hex_str = __rand_hex_str(hexes_num);                                                                 \
        lp_uint_from_hex(val,original_hex_str);                                                                             \
        char *converted_hex_str = lp_uint_to_hex(val);                                                                      \
        if(!__hexcmp(converted_hex_str,original_hex_str))                                                                   \
        {                                                                                                                   \
            printf("Expected: %s, got: %s\n",original_hex_str,converted_hex_str);                                           \
            return false;                                                                                                   \
        }                                                                                                                   \
        free(original_hex_str);                                                                                             \
        free(converted_hex_str);                                                                                            \
    }                                                                                                                       \
    return true;                                                                                                            \
}                                                                                                                           \
bool test_uint##N##_from_to_hex_overflow()                                                                                  \
{                                                                                                                           \
    srand(0);                                                                                                               \
    const uint32_t tests_num = 1000;                                                                                        \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                  \
    {                                                                                                                       \
        uint(N) val;                                                                                                        \
        size_t hexes_num = N*__LP_UINT_HEXES_PER_WORD/__LP_UINT_BITS_PER_WORD;                                              \
        size_t hexes_num_overflowed = hexes_num + rand()%(N/__LP_UINT_BITS_PER_WORD*4);                                     \
        char *original_hex_str = __rand_hex_str(hexes_num_overflowed);                                                      \
        lp_uint_from_hex(val,original_hex_str);                                                                             \
        char *converted_hex_str = lp_uint_to_hex(val);                                                                      \
        if(!__hexcmp(converted_hex_str,original_hex_str+(hexes_num_overflowed-hexes_num)))                                  \
        {                                                                                                                   \
            printf("Expected: %s, got: %s\n",original_hex_str+(hexes_num_overflowed-hexes_num),converted_hex_str);          \
            return false;                                                                                                   \
        }                                                                                                                   \
        free(original_hex_str);                                                                                             \
        free(converted_hex_str);                                                                                            \
    }                                                                                                                       \
    return true;                                                                                                            \
}


TEST_UINT_FROM_TO_HEX(64)
TEST_UINT_FROM_TO_HEX(256)
TEST_UINT_FROM_TO_HEX(1024)


#define TEST_UINT_OPS(N_a, N_b, N_result)                                                                                   \
bool test_uint_##N_a##_##N_b##_##N_result##_addition()                                                                      \
{                                                                                                                           \
    FILE *f_add = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_addition.txt","r");       \
    if(!f_add)                                                                                                              \
        return false;                                                                                                       \
    FILE *f_samp = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt","r");               \
    if(!f_samp)                                                                                                             \
        return false;                                                                                                       \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    char *hex_str_result_true = (char*)malloc(                                                                              \
        __LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+2);                             \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        uint(N_a) a;                                                                                                        \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        uint(N_b) b;                                                                                                        \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        uint(N_result) res_obt, res_true;                                                                                   \
        if(!lp_uint_add(a,b,res_obt))                                                                                       \
            return false;                                                                                                   \
        if(fscanf(f_add,"%s\n",hex_str_result_true) == EOF)                                                                 \
            return false;                                                                                                   \
        if(!lp_uint_from_hex(res_true,hex_str_result_true))                                                                 \
            return false;                                                                                                   \
        if(!lp_uint_eq(res_obt,res_true))                                                                                   \
        {                                                                                                                   \
            char *hex_str_result_obt = lp_uint_to_hex(res_obt);                                                             \
            printf("(+) Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);                                   \
            free(hex_str_result_obt);                                                                                       \
            return false;                                                                                                   \
        }                                                                                                                   \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_add);                                                                                                          \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    free(hex_str_result_true);                                                                                              \
    return true;                                                                                                            \
}                                                                                                                           \
bool test_uint_##N_a##_##N_b##_##N_result##_subtraction()                                                                   \
{                                                                                                                           \
    FILE *f_sub = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_subtraction.txt","r");    \
    if(!f_sub)                                                                                                              \
        return false;                                                                                                       \
    FILE *f_samp = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt","r");               \
    if(!f_samp)                                                                                                             \
        return false;                                                                                                       \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    char *hex_str_result_true = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(2048)+1);                                \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        uint(N_a) a;                                                                                                        \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        uint(N_b) b;                                                                                                        \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        uint(N_result) res_obt, res_true;                                                                                   \
        if(!lp_uint_sub(a,b,res_obt))                                                                                       \
            return false;                                                                                                   \
        if(fscanf(f_sub,"%s\n",hex_str_result_true) == EOF)                                                                 \
            return false;                                                                                                   \
        if(!lp_uint_from_hex(res_true,hex_str_result_true))                                                                 \
            return false;                                                                                                   \
        if(!lp_uint_eq(res_obt,res_true))                                                                                   \
        {                                                                                                                   \
            char *hex_str_result_obt = lp_uint_to_hex(res_obt);                                                             \
            printf("(-) Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);                                   \
            free(hex_str_result_obt);                                                                                       \
            return false;                                                                                                   \
        }                                                                                                                   \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_sub);                                                                                                          \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    free(hex_str_result_true);                                                                                              \
    return true;                                                                                                            \
}                                                                                                                           \
bool test_uint_##N_a##_##N_b##_##N_result##_multiplication()                                                                \
{                                                                                                                           \
    FILE *f_mul = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_multiplication.txt","r"); \
    if(!f_mul)                                                                                                              \
        return false;                                                                                                       \
    FILE *f_samp = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt","r");               \
    if(!f_samp)                                                                                                             \
        return false;                                                                                                       \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    char *hex_str_result_true = (char*)malloc(                                                                              \
        __LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+2);                             \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        uint(N_a) a;                                                                                                        \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        uint(N_b) b;                                                                                                        \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        uint(N_result) res_obt, res_true;                                                                                   \
        if(!lp_uint_mul(a,b,res_obt))                                                                                       \
            return false;                                                                                                   \
        if(fscanf(f_mul,"%s\n",hex_str_result_true) == EOF)                                                                 \
            return false;                                                                                                   \
        if(!lp_uint_from_hex(res_true,hex_str_result_true))                                                                 \
            return false;                                                                                                   \
        if(!lp_uint_eq(res_obt,res_true))                                                                                   \
        {                                                                                                                   \
            char *hex_str_result_obt = lp_uint_to_hex(res_obt);                                                             \
            printf("(*) Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);                                   \
            free(hex_str_result_obt);                                                                                       \
            return false;                                                                                                   \
        }                                                                                                                   \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_mul);                                                                                                          \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    free(hex_str_result_true);                                                                                              \
    return true;                                                                                                            \
}                                                                                                                           \
bool test_uint_##N_a##_##N_b##_##N_result##_comparison()                                                                    \
{                                                                                                                           \
    FILE *f_comp = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b "_comparison.txt","r");    \
    if(!f_comp)                                                                                                             \
        return false;                                                                                                       \
    FILE *f_samp = fopen("/home/me/Documents/Code/lockpick/tests/lp_uint/cases/lp_uint_" #N_a "_" #N_b ".txt","r");               \
    if(!f_samp)                                                                                                             \
        return false;                                                                                                       \
    char *hex_str_a = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_a)+1);                                           \
    char *hex_str_b = (char*)malloc(__LP_UINT_MAX_HEX_STR_REPRESENTATION(N_b)+1);                                           \
    lp_uint_3way_t res_true,res_obt;                                                                                        \
    int32_t res_true_int;                                                                                                   \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        uint(N_a) a;                                                                                                        \
        lp_uint_from_hex(a,hex_str_a);                                                                                      \
        uint(N_b) b;                                                                                                        \
        lp_uint_from_hex(b,hex_str_b);                                                                                      \
        if(fscanf(f_comp,"%d\n",&res_true_int) == EOF)                                                                      \
            return false;                                                                                                   \
        res_true = res_true_int;                                                                                            \
        res_obt = lp_uint_3way(a,b);                                                                                        \
        if(res_true != res_obt)                                                                                             \
        {                                                                                                                   \
            printf("a: %s, b: %s\nExpected comparison res: %d, got: %d",hex_str_a,hex_str_b,res_true_int,(int32_t)res_obt); \
            return false;                                                                                                   \
        }                                                                                                                   \
        if(lp_uint_eq(a,b) && res_true != LP_UINT_EQUAL)                                                                    \
        {                                                                                                                   \
            printf("a: %s, b: %s\nExpected %d, got: 'equal'",hex_str_a,hex_str_b,res_true_int);                             \
            return false;                                                                                                   \
        }                                                                                                                   \
        if(lp_uint_ls(a,b) && res_true != LP_UINT_LESS)                                                                     \
        {                                                                                                                   \
            printf("a: %s, b: %s\nExpected %d, got: 'less'",hex_str_a,hex_str_b,res_true_int);                              \
            return false;                                                                                                   \
        }                                                                                                                   \
        if(lp_uint_gt(a,b) && res_true != LP_UINT_GREATER)                                                                  \
        {                                                                                                                   \
            printf("a: %s, b: %s\nExpected %d, got: 'greater'",hex_str_a,hex_str_b,res_true_int);                           \
            return false;                                                                                                   \
        }                                                                                                                   \
        if(lp_uint_geq(a,b) && res_true == LP_UINT_LESS)                                                                    \
        {                                                                                                                   \
            printf("a: %s, b: %s\nExpected %d, got: 'greater-equal'",hex_str_a,hex_str_b,res_true_int);                     \
            return false;                                                                                                   \
        }                                                                                                                   \
        if(lp_uint_leq(a,b) && res_true == LP_UINT_GREATER)                                                                 \
        {                                                                                                                   \
            printf("a: %s, b: %s\nExpected %d, got: 'less-equal'",hex_str_a,hex_str_b,res_true_int);                        \
            return false;                                                                                                   \
        }                                                                                                                   \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_comp);                                                                                                         \
    free(hex_str_a);                                                                                                        \
    free(hex_str_b);                                                                                                        \
    return true;                                                                                                            \
}                                                                                                                           \
bool test_uint_##N_a##_##N_b##_##N_result##_ops()                                                                           \
{                                                                                                                           \
    if(!test_uint_##N_a##_##N_b##_##N_result##_addition())                                                                  \
        return false;                                                                                                       \
    if(!test_uint_##N_a##_##N_b##_##N_result##_subtraction())                                                               \
        return false;                                                                                                       \
    if(!test_uint_##N_a##_##N_b##_##N_result##_multiplication())                                                            \
        return false;                                                                                                       \
    if(!test_uint_##N_a##_##N_b##_##N_result##_comparison())                                                                \
        return false;                                                                                                       \
    return true;                                                                                                            \
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


void test_uint()
{
    assert(test_uint64_from_to_hex());
    assert(test_uint64_from_to_hex_overflow());

    assert(test_uint256_from_to_hex());
    assert(test_uint256_from_to_hex_overflow());

    assert(test_uint1024_from_to_hex());
    assert(test_uint1024_from_to_hex_overflow());


    assert(test_uint_64_64_256_ops());
    assert(test_uint_64_64_64_ops());

    assert(test_uint_64_128_256_ops());
    assert(test_uint_64_128_128_ops());
    assert(test_uint_64_128_64_ops());

    assert(test_uint_64_1024_1024_ops());
    assert(test_uint_64_1024_256_ops());
    assert(test_uint_64_1024_64_ops());

    assert(test_uint_256_256_1024_ops());
    assert(test_uint_256_256_512_ops());
    assert(test_uint_256_256_256_ops());
    assert(test_uint_256_256_64_ops());

    assert(test_uint_256_1024_1024_ops());
    assert(test_uint_256_1024_256_ops());
    assert(test_uint_256_1024_64_ops());

    assert(test_uint_1024_128_1024_ops());
    assert(test_uint_1024_128_128_ops());
    assert(test_uint_1024_128_64_ops());

    assert(test_uint_1024_256_1024_ops());
    assert(test_uint_1024_256_256_ops());
    assert(test_uint_1024_256_64_ops());

    assert(test_uint_1024_1024_2048_ops());
    assert(test_uint_1024_1024_1024_ops());
    assert(test_uint_1024_1024_64_ops());
}