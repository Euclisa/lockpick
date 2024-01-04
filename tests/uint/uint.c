#include <uint.h>
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

#define TEST_UINT_FROM_TO_HEX(N)                                                                \
bool test_uint##N##_from_to_hex()                                                               \
{                                                                                               \
    srand(0);                                                                                   \
    const uint32_t tests_num = 1000;                                                            \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                      \
    {                                                                                           \
        uint(N) val;                                                                            \
        size_t hexes_num = N*HEXES_PER_WORD/BITS_PER_WORD;                                      \
        char *original_hex_str = __rand_hex_str(hexes_num);                                     \
        uint_from_hex(&val,original_hex_str);                                                   \
        char *converted_hex_str = uint_to_hex(&val);                                            \
        if(!__hexcmp(converted_hex_str,original_hex_str))                                       \
        {                                                                                       \
            printf("Expected: %s, got: %s\n",original_hex_str,converted_hex_str);               \
            return false;                                                                       \
        }                                                                                       \
        free(original_hex_str);                                                                 \
        free(converted_hex_str);                                                                \
    }                                                                                           \
    return true;                                                                                \
}


#define TEST_UINT_FROM_TO_HEX_OVERFLOW(N)                                                                                   \
bool test_uint##N##_from_to_hex_overflow()                                                                                  \
{                                                                                                                           \
    srand(0);                                                                                                               \
    const uint32_t tests_num = 1000;                                                                                        \
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)                                                                  \
    {                                                                                                                       \
        uint(N) val;                                                                                                        \
        size_t hexes_num = N*HEXES_PER_WORD/BITS_PER_WORD;                                                                  \
        size_t hexes_num_overflowed = hexes_num + rand()%(N/BITS_PER_WORD*4);                                               \
        char *original_hex_str = __rand_hex_str(hexes_num_overflowed);                                                      \
        uint_from_hex(&val,original_hex_str);                                                                               \
        char *converted_hex_str = uint_to_hex(&val);                                                                        \
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


TEST_UINT_FROM_TO_HEX(16)
TEST_UINT_FROM_TO_HEX_OVERFLOW(16)

TEST_UINT_FROM_TO_HEX(64)
TEST_UINT_FROM_TO_HEX_OVERFLOW(64)

TEST_UINT_FROM_TO_HEX(1024)
TEST_UINT_FROM_TO_HEX_OVERFLOW(1024)


#define TEST_UINT_ADDITION(N_a, N_b, N_result)                                                                              \
bool test_uint_##N_a##_##N_b##_##N_result##_addition()                                                                      \
{                                                                                                                           \
    FILE *f_add = fopen("/home/me/Documents/Code/lockpick/tests/uint/cases/uint_" #N_a "_" #N_b "_addition.txt","r");                        \
    if(!f_add)                                                                                                              \
        return false;                                                                                                       \
    FILE *f_samp = fopen("/home/me/Documents/Code/lockpick/tests/uint/cases/uint_" #N_a "_" #N_b ".txt","r");                                \
    if(!f_samp)                                                                                                             \
        return false;                                                                                                       \
    char *hex_str_a = (char*)malloc(__UINT_MAX_HEX_STR_REPRESENTATION(N_a));                                                \
    char *hex_str_b = (char*)malloc(__UINT_MAX_HEX_STR_REPRESENTATION(N_b));                                                \
    char *hex_str_result_true = (char*)malloc(                                                                              \
        __UINT_MAX_HEX_STR_REPRESENTATION(N_a)+__UINT_MAX_HEX_STR_REPRESENTATION(N_b)+2);                                   \
    while(fscanf(f_samp,"%s %s\n",hex_str_a,hex_str_b) != EOF)                                                              \
    {                                                                                                                       \
        uint(N_a) a;                                                                                                        \
        uint_from_hex(&a,hex_str_a);                                                                                        \
        uint(N_b) b;                                                                                                        \
        uint_from_hex(&b,hex_str_b);                                                                                        \
        uint(N_result) res_obt, res_true;                                                                                   \
        if(!uint_add(&a,&b,&res_obt))                                                                                       \
            return false;                                                                                                   \
        if(fscanf(f_add,"%s\n",hex_str_result_true) == EOF)                                                                 \
            return false;                                                                                                   \
        if(!uint_from_hex(&res_true,hex_str_result_true))                                                                   \
            return false;                                                                                                   \
        if(!uint_eq(&res_obt,&res_true))                                                                                    \
        {                                                                                                                   \
            char *hex_str_result_obt = uint_to_hex(&res_obt);                                                               \
            printf("Expected: %s, got: %s\n",hex_str_result_true,hex_str_result_obt);                                       \
            return false;                                                                                                   \
        }                                                                                                                   \
    }                                                                                                                       \
    fclose(f_samp);                                                                                                         \
    fclose(f_add);                                                                                                          \
    return true;                                                                                                            \
}

TEST_UINT_ADDITION(16,16,256)
TEST_UINT_ADDITION(16,16,16)

TEST_UINT_ADDITION(16,32,256)
TEST_UINT_ADDITION(16,32,32)
TEST_UINT_ADDITION(16,32,16)

TEST_UINT_ADDITION(16,1024,256)
TEST_UINT_ADDITION(16,1024,16)
TEST_UINT_ADDITION(16,1024,1024)

TEST_UINT_ADDITION(256,256,1024)
TEST_UINT_ADDITION(256,256,256)
TEST_UINT_ADDITION(256,256,16)

TEST_UINT_ADDITION(256,1024,1024)
TEST_UINT_ADDITION(256,1024,256)
TEST_UINT_ADDITION(256,1024,16)


void test_uint()
{
    assert(test_uint16_from_to_hex());
    assert(test_uint16_from_to_hex_overflow());

    assert(test_uint64_from_to_hex());
    assert(test_uint64_from_to_hex_overflow());

    assert(test_uint1024_from_to_hex());
    assert(test_uint1024_from_to_hex_overflow());


    assert(test_uint_16_16_256_addition());
    assert(test_uint_16_16_16_addition());

    assert(test_uint_16_32_256_addition());
    assert(test_uint_16_32_32_addition());
    assert(test_uint_16_32_16_addition());

    assert(test_uint_16_1024_1024_addition());
    assert(test_uint_16_1024_256_addition());
    assert(test_uint_16_1024_16_addition());

    assert(test_uint_256_256_1024_addition());
    assert(test_uint_256_256_256_addition());
    assert(test_uint_256_256_16_addition());

    assert(test_uint_256_1024_1024_addition());
    assert(test_uint_256_1024_256_addition());
    assert(test_uint_256_1024_16_addition());
}