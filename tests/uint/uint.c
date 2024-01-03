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


bool __test_uint_from_to_hex(uint16_t *value, size_t value_size)
{
    srand(0);
    const uint32_t tests_num = 1000;
    
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        size_t hexes_num = (value_size*HEXES_PER_WORD);
        char *original_hex_str = __rand_hex_str(hexes_num);
        __uint_from_hex(original_hex_str,value,value_size);
        char *converted_hex_str = __uint_to_hex(value,value_size);
        if(!__hexcmp(converted_hex_str,original_hex_str))
        {
            printf("Expected: %s, got: %s\n",original_hex_str,converted_hex_str);
            return false;
        }
        free(original_hex_str);
        free(converted_hex_str);
    }

    return true;
}

#define test_uint_from_to_hex(value) __test_uint_from_to_hex((value)->__buffer, __array_size((value)->__buffer))


bool __test_uint_from_to_hex_overflow(uint16_t *value, size_t value_size)
{
    srand(0);
    const uint32_t tests_num = 1000;
    
    for(uint32_t test_i = 0; test_i < tests_num; ++test_i)
    {
        size_t hexes_num = (value_size*HEXES_PER_WORD);
        size_t hexes_num_overflowed = hexes_num + rand()%value_size;
        const char *original_hex_str = __rand_hex_str(hexes_num_overflowed);
        __uint_from_hex(original_hex_str,value,value_size);
        const char *converted_hex_str = __uint_to_hex(value,value_size);
        if(!__hexcmp(converted_hex_str,original_hex_str+(hexes_num_overflowed-hexes_num)))
        {
            printf("Expected: %s, got: %s\n",original_hex_str+(hexes_num_overflowed-hexes_num),converted_hex_str);
            return false;
        }
        free(original_hex_str);
        free(converted_hex_str);
    }

    return true;
}

#define test_uint_from_to_hex_overflow(value) __test_uint_from_to_hex_overflow((value)->__buffer, __array_size((value)->__buffer))


void test_uint()
{
    uint(16) u16;
    assert(test_uint_from_to_hex(&u16));
    assert(test_uint_from_to_hex_overflow(&u16));
    uint(64) u64;
    assert(test_uint_from_to_hex(&u64));
    assert(test_uint_from_to_hex_overflow(&u64));
    uint(1024) u1024;
    assert(test_uint_from_to_hex(&u1024));
    assert(test_uint_from_to_hex_overflow(&u1024));
}