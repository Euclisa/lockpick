#include <lockpick/graph/types/uint.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <string.h>
#include <stdio.h>

#define __LPG_UINT_BAD_CHAR 255

#define __LPG_UINT_NODES_MASK ((uintptr_t)(~0b1))
#define __LPG_UINT_OWN_MASK ((uintptr_t)0b1)


inline lpg_node_t **lpg_uint_nodes(const lpg_uint_t *value)
{
    return (lpg_node_t **)(value->__nodes_own & __LPG_UINT_NODES_MASK);
}

static inline void __lpg_uint_set_nodes(lpg_uint_t *value, lpg_node_t **nodes)
{
    value->__nodes_own = (uintptr_t)nodes | (value->__nodes_own & 0b1);
}


static inline bool __lpg_uint_own(const lpg_uint_t *value)
{
    return value->__nodes_own & __LPG_UINT_OWN_MASK;
}


static inline void __lpg_uint_set_own(lpg_uint_t *value, bool own)
{
    if(own)
        value->__nodes_own |= __LPG_UINT_OWN_MASK;
    else
        value->__nodes_own &= ~__LPG_UINT_OWN_MASK;
}


static inline lpg_uint_t *__lpg_uint_general_init(lpg_graph_t *graph, size_t width)
{
    lpg_uint_t *_uint = (lpg_uint_t*)malloc(sizeof(lpg_uint_t));
    affirmf(_uint,"Failed to allocate space for 'lpg_uint_t' object");

    _uint->graph = graph;
    _uint->width = width;

    return _uint;
}


lpg_uint_t *lpg_uint_create_from_nodes(lpg_graph_t *graph, lpg_node_t **nodes, size_t width)
{
    affirmf(graph && nodes,"Expected valid pointer but null was given");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    lpg_node_t **nodes_buff = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*width);
    affirmf(nodes_buff,"Failed to allocate buffer for nodes");
    __lpg_uint_set_nodes(_uint,nodes_buff);
    __lpg_uint_set_own(_uint,true);

    for(size_t node_i = 0; node_i < width; ++node_i)
        nodes_buff[node_i] = nodes[node_i];

    return _uint;
}


lpg_uint_t *lpg_uint_create_from_hex_str(lpg_graph_t *graph, const char *hex_str, size_t width)
{
    affirmf(graph,"Expected valid pointer but null was given");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    lpg_node_t **nodes_buff = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*width);
    affirmf(nodes_buff,"Failed to allocate buffer for nodes");
    __lpg_uint_set_nodes(_uint,nodes_buff);
    __lpg_uint_set_own(_uint,true);

    for(size_t node_i = 0; node_i < width; ++node_i)
    {
        nodes_buff[node_i] = lpg_node_var(graph);
        __lpg_node_set_computed(nodes_buff[node_i],true);
    }
    
    lpg_uint_update_from_hex(hex_str,_uint);

    return _uint;
}


lpg_uint_t *lpg_uint_create_as_view(lpg_graph_t *graph, lpg_node_t **nodes, size_t width)
{
    affirmf(graph && nodes,"Expected valid pointer but null was given");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    __lpg_uint_set_own(_uint,false);

    __lpg_uint_set_nodes(_uint,nodes);

    return _uint;
}


void lpg_uint_release(lpg_uint_t *_uint)
{
    bool own_buffer = __lpg_uint_own(_uint);
    if(own_buffer)
    {
        lpg_node_t **nodes_buff = lpg_uint_nodes(_uint);
        free(nodes_buff);
    }

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
    return __LPG_UINT_BAD_CHAR;
}


void lpg_uint_update_from_hex(const char *hex_str, lpg_uint_t *value)
{
    affirmf(hex_str && value,"Expected valid pointer but null was given");

    lpg_node_t **nodes = lpg_uint_nodes(value);

    size_t hex_str_len = strlen(hex_str);
    size_t upper_bound = MIN(value->width,hex_str_len*__LPG_UINT_BITS_PER_HEX);
    for(size_t node_i = 0; node_i < upper_bound; node_i += __LPG_UINT_BITS_PER_HEX)
    {
        size_t curr_hex_i = hex_str_len - node_i/__LPG_UINT_BITS_PER_HEX - 1;
        uint8_t curr_hex = __lpg_uint_ch2i(hex_str[curr_hex_i]);
        affirmf(curr_hex != __LPG_UINT_BAD_CHAR,"Unexpected character '%c' inside hex-string",hex_str[curr_hex_i]);

        for(size_t bit_offset = 0; bit_offset < __LPG_UINT_BITS_PER_HEX; ++bit_offset)
        {
            size_t node_i_off = node_i+bit_offset;
            if(node_i_off >= upper_bound)
                goto end_for;

            bool curr_bit_value = (curr_hex >> bit_offset) & 0b1;
            __lpg_node_set_value(nodes[node_i_off],curr_bit_value);
        }
    }
    end_for:

    for(size_t node_i = upper_bound; node_i < value->width; ++node_i)
        __lpg_node_set_value(nodes[node_i],false);
}


void __lpg_uint_update_from_uint(lpg_uint_t *value, const __lp_uint_word_t *uint_value, size_t uint_value_size)
{
    affirmf(value && uint_value,"Expected valid pointer but null was given");

    lpg_node_t **nodes = lpg_uint_nodes(value);

    size_t uint_value_width = uint_value_size*__LP_UINT_BITS_PER_WORD;
    size_t upper_bound = MIN(value->width,uint_value_width);
    size_t node_i = 0;
    for(; node_i < upper_bound; ++node_i)
    {
        size_t curr_uint_word_i = node_i / __LP_UINT_BITS_PER_WORD;
        size_t curr_uint_bit_i = node_i % __LP_UINT_BITS_PER_WORD;

        bool curr_bit_value = (uint_value[curr_uint_word_i] >> curr_uint_bit_i) & 0b1;
        __lpg_node_set_value(nodes[node_i],curr_bit_value);
    }

    for(; node_i < value->width; ++node_i)
        __lpg_node_set_value(nodes[node_i],false);
}


char __lpg_uint_i2ch(uint8_t value)
{
    static const char i2ch_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    affirmf_debug(value < sizeof(i2ch_map),"Hex value out of range");

    return i2ch_map[value];
}


size_t lpg_uint_to_hex(const lpg_uint_t *value, char *dest, size_t n)
{
    affirmf(value,"Expected valid pointer of type 'lpg_uint_t' but null was given");

    lpg_node_t **nodes = lpg_uint_nodes(value);

    // Find highest non-zero bit
    int64_t significant_bits_offset = value->width-1;
    for(; significant_bits_offset >= 0 && !lpg_node_validate_fetch(value->graph,nodes[significant_bits_offset]); --significant_bits_offset);

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
                if(lpg_node_validate_fetch(value->graph,nodes[curr_bit_i]))
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


static inline void __lpg_uint_add_left_smaller(lpg_graph_t *graph, lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    lpg_node_t *carry = lpg_node_const(graph,false);
    size_t a_upper_bound = MIN(a->width,result->width);
    size_t node_i = 0;
    for(; node_i < a_upper_bound; ++node_i)
    {
        lpg_node_t *terms_part = lpg_node_xor(graph,a_nodes[node_i],b_nodes[node_i]);
        result_nodes[node_i] = lpg_node_xor(graph,terms_part,carry);
        carry = lpg_node_or(graph,
                    lpg_node_and(graph,terms_part,carry),
                    lpg_node_and(graph,a_nodes[node_i],b_nodes[node_i])
                );
    }

    size_t b_upper_bound = MIN(b->width,result->width);
    for(; node_i < b_upper_bound; ++node_i)
    {
        result_nodes[node_i] =
                lpg_node_xor(graph,
                    carry,
                    b_nodes[node_i]
                );
        carry =
            lpg_node_and(graph,
                carry,
                b_nodes[node_i]
            );
    }

    if(node_i < result->width)
        result_nodes[node_i] = carry;
    ++node_i;

    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


void lpg_uint_add(lpg_graph_t *graph, lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(graph && a && b && result,"Expected valid pointer but null was given");
    
    if(a->width < b->width)
        __lpg_uint_add_left_smaller(graph,a,b,result);
    else
        __lpg_uint_add_left_smaller(graph,b,a,result);
}


void lpg_uint_sub(lpg_graph_t *graph, lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(graph && a && b && result,"Expected valid pointer but null was given");

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    lpg_node_t *carry = lpg_node_const(graph,false);
    size_t common_upper_bound = MIN(MIN(a->width,b->width),result->width);
    size_t node_i = 0;
    for(; node_i < common_upper_bound; ++node_i)
    {
        lpg_node_t *terms_part = lpg_node_xor(graph,a_nodes[node_i],b_nodes[node_i]);
        result_nodes[node_i] = lpg_node_xor(graph,terms_part,carry);
        carry = lpg_node_or(graph,
                    lpg_node_and(graph,
                        lpg_node_not(graph,terms_part),
                        carry
                    ),
                    lpg_node_and(graph,
                        lpg_node_not(graph,a_nodes[node_i]),
                        b_nodes[node_i]
                    )
                );
    }

    if(a->width < b->width)
    {
        size_t upper_bound = MIN(b->width,result->width);
        for(; node_i < upper_bound; ++node_i)
        {
            result_nodes[node_i] = lpg_node_xor(graph,b_nodes[node_i],carry);
            carry = lpg_node_or(graph,b_nodes[node_i],carry);
        }
    }
    else
    {
        size_t upper_bound = MIN(a->width,result->width);
        for(; node_i < upper_bound; ++node_i)
        {
            result_nodes[node_i] = lpg_node_xor(graph,a_nodes[node_i],carry);
            carry = lpg_node_and(graph,
                        lpg_node_not(graph,a_nodes[node_i]),
                        carry
                    );
        }
    }

    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = carry;
}