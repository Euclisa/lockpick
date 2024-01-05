#include "uint.h"
#include <string.h>
#include <stdlib.h>

#define BAD_CHAR 255

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define HEXES_PER_WORD (sizeof(uint16_t)*2)
#define BITS_PER_HEX 4
#define BITS_PER_WORD (sizeof(uint16_t)*8)


const uint32_t __base = (1 << (sizeof(uint16_t)*8));
const uint16_t __max_word = 0xffff;


/**
 * __uint_ch2i - converts hex characters to unsigned integer
 * @char_hex:	hex character ([0-9a-fA-F])
 * 
 * Returns unsigned integer corresponding to the given character.
 * BAD_CHAR if @char_hex is invalid.
 */
uint8_t __uint_ch2i(char char_hex)
{
    if(char_hex >= '0' && char_hex <= '9')
        return char_hex - '0';
    if(char_hex >= 'A' && char_hex <= 'F')
        return char_hex - 'A' + 10;
    if(char_hex >= 'a' && char_hex <= 'f')
        return char_hex - 'a' + 10;
    return BAD_CHAR;
}


/**
 * __uint_parse_16_bits_reverse - parses 16 bits (2 bytes) in reverse order starting at given position
 * @hex_str:	string containing hex number
 * @start:      index of the character to start from
 * @result:     pointer on a word where to put the result
 * 
 * Returns number of character parsed.
 * -1 if invalid character was found.
 */
int8_t __uint_parse_16_bits_reverse(const char *hex_str, uint32_t start, uint16_t *result)
{
    *result = 0;
    int64_t curr_char_i = start;
    
    // Characters are parsed in reverse order so low hexes come first
    for (uint8_t offset = 0; offset < HEXES_PER_WORD*BITS_PER_HEX; offset += 4)
    {
        // If nothing left to parse
        if (curr_char_i < 0)
            break;

        char curr_char = hex_str[curr_char_i];
        uint8_t char_converted = __uint_ch2i(curr_char);
        if(char_converted == BAD_CHAR)
            return -1;

        *result += char_converted << offset;
        --curr_char_i;
    }

    return start - curr_char_i;
}


/**
 * __uint_from_hex - initializes uint object from hex string
 * @hex_str:	string containing hex number
 * @value:      pointer on uint buffer
 * @value_size: size of uint buffer
 * 
 * Returns status of initialization.
 * 
 * This is not supposed to be called by user. Use 'uint_from_hex' macro instead.
 */
bool __uint_from_hex(const char *hex_str, uint16_t *value, size_t value_size)
{
    size_t hex_str_len = strlen(hex_str);
    int64_t curr_char_i = hex_str_len-1;
    size_t curr_word_i = 0;
    while(curr_word_i < value_size && curr_char_i >= 0)
    {
        int8_t read_hexes = __uint_parse_16_bits_reverse(hex_str, curr_char_i, value+curr_word_i);
        if(read_hexes < 0)
            return false;
        curr_char_i -= read_hexes;
        ++curr_word_i;
    }

    for(; curr_word_i < value_size; value[curr_word_i++] = 0);

    return true;
}


/**
 * __uint_i2ch - converts single word to hex string
 * @value:          word to convert
 * @dest:           destination pointer on string to store result in
 * @truncate_zeros: whether to truncate high zeros or not
 * 
 * Returns number of characters written.
 */
uint8_t __uint_i2ch(uint16_t value, char *dest, bool truncate_zeros)
{
    static const char i2ch_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    uint8_t wrote_hexes = 0;
    size_t offset = 0;
    for(int8_t shift = 12; shift >= 0; shift -= 4)
    {
        uint16_t shifted = value >> shift;
        if(truncate_zeros && shifted == 0)
            continue;
        char shifted_hex = i2ch_map[shifted & 0b1111];
        dest[offset++] = shifted_hex;
        ++wrote_hexes;
    }

    return wrote_hexes;
}


/**
 * __uint_to_hex - converts uint object to hex string
 * @value:      pointer on uint buffer
 * @value_size: size of uint buffer
 * 
 * Returns pointer on hex string.
 * NULL if @value is NULL.
 * 
 * This is not supposed to be called by user. Use 'uint_to_hex' macro instead.
 */
char *__uint_to_hex(uint16_t *value, size_t value_size)
{
    if(!value)
        return NULL;

    // Find first non-zero word (from the end)
    int64_t significant_words_offset = value_size-1;
    for(; significant_words_offset >= 0 && value[significant_words_offset] == 0; --significant_words_offset);

    // If all words == 0
    if(significant_words_offset < 0)
        return "0";

    // All high zeros in the highest word must be truncated
    size_t hex_str_len = (significant_words_offset) * HEXES_PER_WORD;
    for(uint8_t shift = 0; shift < HEXES_PER_WORD*BITS_PER_HEX && (value[significant_words_offset] >> shift); shift += BITS_PER_HEX)
        ++hex_str_len;

    char *hex_str = (char*)malloc(hex_str_len+1);
    hex_str[hex_str_len] = '\0';

    size_t wrote_hexes = 0;
    wrote_hexes += __uint_i2ch(value[significant_words_offset--],hex_str+wrote_hexes,true);
    for(; significant_words_offset != -1; --significant_words_offset)
        wrote_hexes += __uint_i2ch(value[significant_words_offset],hex_str+wrote_hexes,false);
    
    return hex_str;
}


bool __uint_add(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size, uint16_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return false;
    if(a == result || b == result)
        return false;
    
    uint16_t *min_term, *max_term;
    size_t min_term_size, max_term_size;
    if(a_size < b_size)
    {
        min_term = a;
        min_term_size = a_size;
        max_term = b;
        max_term_size = b_size;
    }
    else
    {
        min_term = b;
        min_term_size = b_size;
        max_term = a;
        max_term_size = a_size;
    }

    uint32_t carry = 0;
    size_t min_term_upper_bound = MIN(min_term_size,result_size);
    for(size_t word_i = 0; word_i < min_term_upper_bound; ++word_i)
    {
        uint32_t curr_sum = min_term[word_i] + max_term[word_i] + carry;
        result[word_i] = curr_sum & __max_word;
        carry = curr_sum >> BITS_PER_WORD;
    }

    size_t max_term_upper_bound = MIN(max_term_size,result_size);
    for(size_t word_i = min_term_upper_bound; word_i < max_term_upper_bound; ++word_i)
    {
        uint32_t curr_sum = max_term[word_i] + carry;
        result[word_i] = curr_sum & __max_word;
        carry = curr_sum >> BITS_PER_WORD;
    }
    
    if(result_size != max_term_upper_bound)
    {
        result[max_term_upper_bound] = carry;
        for(size_t word_i = max_term_upper_bound+1; word_i < result_size; ++word_i)
            result[word_i] = 0;
    }

    return true;
}


bool __uint_sub(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size, uint16_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return false;
    if(a == result || b == result)
        return false;

    uint32_t carry = 0;
    size_t common_upper_bound = MIN(MIN(a_size,b_size),result_size);
    size_t word_i = 0;
    /*
        Subtract common part normally saving carry for future:
        a0 a1 a2 a3 ... -> carry
        b0 b1 b2 b3 ...
                  ^
    */
    for(; word_i < common_upper_bound; ++word_i)
    {
        result[word_i] = a[word_i];
        uint32_t total_neg = b[word_i] + carry;
        if(a[word_i] < total_neg)
        {
            result[word_i] += __base - total_neg;
            carry = 1;
        }
        else
        {
            result[word_i] -= total_neg;
            carry = 0;
        }
    }

    if(a_size < b_size)
    {
        size_t b_upper_bound = MIN(b_size,result_size);
        /*
            If b is of greater width than a, then if carry is 1 then it always will be 1.
            That is why we first skip all zero b words if carry is zero.

            a0 a1 a2 a3 0 0 ... 0   0    ...
            b0 b1 b2 b3 0 0 ... bk  bk+1 ...
                      ^         ^
                  begin         end (bk != 0)
        */
        for(; (word_i < b_upper_bound) && (b[word_i] + carry == 0); ++word_i)
            result[word_i] = 0;

        if(word_i < b_upper_bound)
        {
            /*
                If not all excess words of b were zero then we perform subtraction of bk from corresponding zero ak word.
                After this carry is always one.
            */
            result[word_i] = __base - (b[word_i] + carry);  // Either carry (then first part was skipped) or bk is not zero now
            carry = 1;
            ++word_i;
            for(; word_i < b_upper_bound; ++word_i)
                result[word_i] = __base - (b[word_i] + carry); // carry and __base are optimized
        }
    }
    else
    {
        size_t a_upper_bound = MIN(a_size,result_size);
        /*
            If a is of greater width than b, then if carry is 0 then it always will be 0.
            That is why we first skip all zero a words if carry is not zero (ai < carry is sufficient for that).

            a0 a1 a2 a3 0 0 ... ak  ak+1 ...
            b0 b1 b2 b3 0 0 ... 0   0    ...
                      ^         ^
                  begin         end (bk != 0)
        */
        for(; (word_i < a_upper_bound) && (a[word_i] < carry); ++word_i)
            result[word_i] = __base - 1;
        
        if(word_i < a_upper_bound)
        {
            /*
                If not all excess words of a were zero then we perform subtraction of carry from ak.
                From now on carry is zero.
            */
            result[word_i] = a[word_i] - carry;
            carry = 0;
            ++word_i;
            for(; word_i < a_upper_bound; ++word_i)
                result[word_i] = a[word_i];
        }
    }
    
    /*
        If result width is greater than either a and b then excess words are either 0xffff or 0 depending on carry only.
    */
    for(; word_i < result_size; ++word_i)
        result[word_i] = __base - carry;

    return true;
}


bool __uint_mul(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size, uint16_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return false;
    if(a == result || b == result)
        return false;
    
    uint64_t carry = 0;
    size_t a_last_i = a_size - 1;
    size_t b_last_i = b_size - 1;
    size_t result_upper_bound = MIN(a_last_i+b_last_i,result_size-1);
    size_t res_i = 0;
    for(; res_i <= result_upper_bound; ++res_i)
    {
        size_t a_lower_bound = res_i > b_last_i ? res_i - b_last_i : 0;
        size_t a_upper_bound = MIN(a_last_i,res_i);
        uint64_t res_conv = carry;
        for(size_t a_i = a_lower_bound; a_i <= a_upper_bound; ++a_i)
        {
            size_t b_i = res_i-a_i;
            res_conv += (uint64_t)a[a_i]*(uint64_t)b[b_i];
        }
        carry = res_conv >> __UINT_BITS_PER_WORD;
        result[res_i] = res_conv & __max_word;
    }

    for(; res_i < result_size; ++res_i)
    {
        result[res_i] = carry & __max_word;
        carry >>= __UINT_BITS_PER_WORD;
    }

    return true;
}


bool __uint_eq(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size)
{
    if(!a || !b)
        return false;
    
    uint16_t *min_term, *max_term;
    size_t min_term_size, max_term_size;
    if(a_size < b_size)
    {
        min_term = a;
        min_term_size = a_size;
        max_term = b;
        max_term_size = b_size;
    }
    else
    {
        min_term = b;
        min_term_size = b_size;
        max_term = a;
        max_term_size = a_size;
    }

    for(size_t word_i = 0; word_i < min_term_size; ++word_i)
    {
        if(min_term[word_i] != max_term[word_i])
            return false;
    }

    for(size_t word_i = min_term_size; word_i < max_term_size; ++word_i)
    {
        if(max_term[word_i] != 0)
            return false;
    }

    return true;
}


bool __uint_ls(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size)
{
    if(!a || !b)
        return false;
    
    uint16_t *max_term;
    int64_t min_term_size, max_term_size;
    if(a_size < b_size)
    {
        min_term_size = a_size;
        max_term = b;
        max_term_size = b_size;
    }
    else
    {
        min_term_size = b_size;
        max_term = a;
        max_term_size = a_size;
    }

    for(int64_t word_i = max_term_size-1; word_i >= min_term_size; --word_i)
    {
        if(max_term[word_i] != 0)
            return a != max_term;
    }

    for(int64_t word_i = min_term_size-1; word_i >= 0; --word_i)
    {
        if(a[word_i] == b[word_i])
            continue;
        return a[word_i] < b[word_i];
    }

    return false;
}


bool __uint_gt(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size)
{
    if(!a || !b)
        return false;
    
    uint16_t *max_term;
    int64_t min_term_size, max_term_size;
    if(a_size < b_size)
    {
        min_term_size = a_size;
        max_term = b;
        max_term_size = b_size;
    }
    else
    {
        min_term_size = b_size;
        max_term = a;
        max_term_size = a_size;
    }

    for(int64_t word_i = max_term_size-1; word_i >= min_term_size; --word_i)
    {
        if(max_term[word_i] != 0)
            return a == max_term;
    }

    for(int64_t word_i = min_term_size-1; word_i >= 0; --word_i)
    {
        if(a[word_i] == b[word_i])
            continue;
        return a[word_i] > b[word_i];
    }

    return false;
}


bool __uint_leq(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size)
{
    if(!a || !b)
        return false;
    
    uint16_t *max_term;
    int64_t min_term_size, max_term_size;
    if(a_size < b_size)
    {
        min_term_size = a_size;
        max_term = b;
        max_term_size = b_size;
    }
    else
    {
        min_term_size = b_size;
        max_term = a;
        max_term_size = a_size;
    }

    for(int64_t word_i = max_term_size-1; word_i >= min_term_size; --word_i)
    {
        if(max_term[word_i] != 0)
            return a != max_term;
    }

    for(int64_t word_i = min_term_size-1; word_i >= 0; --word_i)
    {
        if(a[word_i] == b[word_i])
            continue;
        return a[word_i] < b[word_i];
    }

    return true;
}


bool __uint_geq(uint16_t *a, size_t a_size, uint16_t *b, size_t b_size)
{
    if(!a || !b)
        return false;
    
    uint16_t *max_term;
    int64_t min_term_size, max_term_size;
    if(a_size < b_size)
    {
        min_term_size = a_size;
        max_term = b;
        max_term_size = b_size;
    }
    else
    {
        min_term_size = b_size;
        max_term = a;
        max_term_size = a_size;
    }

    for(int64_t word_i = max_term_size-1; word_i >= min_term_size; --word_i)
    {
        if(max_term[word_i] != 0)
            return a == max_term;
    }

    for(int64_t word_i = min_term_size-1; word_i >= 0; --word_i)
    {
        if(a[word_i] == b[word_i])
            continue;
        return a[word_i] > b[word_i];
    }

    return true;
}
