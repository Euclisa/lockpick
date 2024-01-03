#include "uint.h"
#include <string.h>
#include <stdlib.h>

#define BAD_CHAR 255

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define HEXES_PER_WORD (sizeof(uint16_t)*2)
#define BITS_PER_HEX 4


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
    while(curr_word_i < value_size)
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
 * __uint_from_hex - converts hex string corrsponding to uint object
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


bool __uint_add(uint16_t *a, uint32_t a_size, uint16_t *b, uint32_t b_size, uint16_t *result, uint32_t result_size)
{

}