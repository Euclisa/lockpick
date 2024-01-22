#include <lockpick/graph/types/uint.h>
#include <lockpick/affirmf.h>
#include <string.h>
#include <stdio.h>

#define __LPG_UINT_BAD_CHAR 255

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


lpg_uint_t *lpg_uint_create(size_t width)
{
    lpg_uint_t *_uint = (lpg_uint_t*)malloc(sizeof(lpg_uint_t));
    affirmf(_uint,"Failed to allocate space for graph uint struct");
    _uint->nodes = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*width);
    affirmf(_uint->nodes,"Failed to allocate space for %ld nodes",width);
    _uint->width = width;

    return _uint;
}


void lpg_uint_release(lpg_uint_t *_uint)
{
    free(_uint->nodes);
    free(_uint);
}

/**
 * __lpg_uint_ch2i - converts hex characters to unsigned integer
 * @char_hex:	hex character ([0-9a-fA-F])
 * 
 * Returns unsigned integer corresponding to the given character.
 * __LPG_UINT_BAD_CHAR if 'char_hex' is invalid.
 */
uint8_t __lpg_uint_ch2i(char char_hex)
{
    if(char_hex >= '0' && char_hex <= '9')
        return char_hex - '0';
    if(char_hex >= 'A' && char_hex <= 'F')
        return char_hex - 'A' + 10;
    if(char_hex >= 'a' && char_hex <= 'f')
        return char_hex - 'a' + 10;
    return_set_errno(__LPG_UINT_BAD_CHAR,EINVAL);
}


bool lpg_uint_from_hex(lpg_graph_t *graph, const char *hex_str, lpg_uint_t *value)
{
    if(!graph || !hex_str || !value)
        return_set_errno(false,EINVAL);

    size_t hex_str_len = strlen(hex_str);
    size_t upper_bound = MIN(value->width,hex_str_len*__LPG_UINT_BITS_PER_HEX);
    for(size_t node_i = 0; node_i < upper_bound; node_i += __LPG_UINT_BITS_PER_HEX)
    {
        size_t curr_hex_i = hex_str_len - node_i/__LPG_UINT_BITS_PER_HEX - 1;
        uint8_t curr_hex = __lpg_uint_ch2i(hex_str[curr_hex_i]);
        if(curr_hex == __LPG_UINT_BAD_CHAR)
            return_set_errno(false,EINVAL);

        for(size_t bit_offset = 0; bit_offset < __LPG_UINT_BITS_PER_HEX; ++bit_offset)
        {
            size_t node_i_off = node_i+bit_offset;
            if(node_i_off >= upper_bound)
                goto end_for;

            bool curr_bit_value = (curr_hex >> bit_offset) & 0b1;
            value->nodes[node_i_off] = lpg_node_const(graph,curr_bit_value);
        }
    }
    end_for:

    for(size_t node_i = upper_bound; node_i < value->width; ++node_i)
        value->nodes[node_i] = lpg_node_const(graph,false);

    return true;
}


char __lpg_uint_i2ch(uint8_t value)
{
    static const char i2ch_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    affirmf_debug(value < sizeof(i2ch_map),"Hex value out of range");

    return i2ch_map[value];
}


int64_t lpg_uint_to_hex(lpg_uint_t *value, char *dest, size_t n)
{
    if(!value)
        return_set_errno(-1,EINVAL);

    // Find highest non-zero bit
    int64_t significant_bits_offset = value->width-1;
    for(; significant_bits_offset >= 0 && value->nodes[significant_bits_offset]->type == LPG_NODE_TYPE_FALSE; --significant_bits_offset);

    // If all bits == 0
    if(significant_bits_offset < 0)
    {
        if(n > 0 && dest)
        {
            dest[0] = '0';
            dest[1] = '\0';
        }

        return 2;
    }

    size_t hex_str_len = significant_bits_offset/__LPG_UINT_BITS_PER_HEX + 1;
    
    if(dest != NULL)
    {
        size_t hex_str_len_truncated = MIN(n,hex_str_len);
        dest[hex_str_len_truncated] = '\0';

        int64_t curr_bit_i = significant_bits_offset;
        size_t curr_hex_i = 0;

        while(curr_hex_i < hex_str_len_truncated)
        {
            uint8_t curr_hex_value = 0;
            int64_t curr_hex_low_bit_i = (curr_bit_i/__LPG_UINT_BITS_PER_HEX)*__LPG_UINT_BITS_PER_HEX;
            for(; curr_bit_i >= curr_hex_low_bit_i; --curr_bit_i)
            {
                if(value->nodes[curr_bit_i]->type == LPG_NODE_TYPE_TRUE)
                {
                    uint8_t offset = curr_bit_i-curr_hex_low_bit_i;
                    curr_hex_value |= 1 << offset;
                }
            }

            dest[curr_hex_i++] = __lpg_uint_i2ch(curr_hex_value);
        }

    }
    
    return hex_str_len;
}
