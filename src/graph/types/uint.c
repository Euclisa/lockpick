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


lpg_uint_t *lpg_uint_allocate(lpg_graph_t *graph, size_t width)
{
    affirmf(graph,"Expected valid graph pointer but null was given");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    lpg_node_t **nodes = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*width);
    affirmf(nodes,"Failed to allocate buffer for nodes");

    for(size_t node_i = 0; node_i < width; ++node_i)
        nodes[node_i] = NULL;

    __lpg_uint_set_nodes(_uint,nodes);
    __lpg_uint_set_own(_uint,true);

    return _uint;
}


lpg_uint_t *lpg_uint_allocate_as_view(lpg_graph_t *graph, lpg_node_t **nodes, size_t width)
{
    affirmf(graph,"Expected valid graph pointer but null was given");
    affirmf(nodes,"Expected valid pointer on nodes buffer but null was given");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    __lpg_uint_set_nodes(_uint,nodes);
    __lpg_uint_set_own(_uint,false);

    return _uint;
}


static inline bool __lpg_uint_is_valid_uint(const lpg_uint_t *value)
{
    if(!value)
        return false;
    if(!value->graph)
        return false;
    if(!lpg_uint_nodes(value))
        return false;
    return true;
}


void lpg_uint_update_from_nodes(lpg_uint_t *value, lpg_node_t **nodes)
{
    affirmf(value,"Expected valid pointer on 'lpg_uint_t' but null was given");
    affirmf(nodes,"Expected valid pointer on nodes buffer but null was given");
    affirmf(__lpg_uint_is_valid_uint(value),"Specified 'lpg_uint_t' object is invalid");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
    {
        affirmf(__lpg_node_belongs_to_graph(value->graph,nodes[node_i]),
        "Node inside buffer at index '%ld' does not belong to the graph given value bounded with");
        value_nodes[node_i] = nodes[node_i];
    }
}


void lpg_uint_update_fill_with_single(lpg_uint_t *value, lpg_node_t *node)
{
    affirmf(value,"Expected valid pointer on 'lpg_uint_t' but null was given");
    affirmf(node,"Expected valid pointer but null was given");
    affirmf(__lpg_node_belongs_to_graph(value->graph,node),"Specified node does not belong to the graph given value bounded with");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        value_nodes[node_i] = node;
}


void lpg_uint_update_empty(lpg_uint_t *value)
{
    affirmf(value,"Expected valid pointer on 'lpg_uint_t' but null was given");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        value_nodes[node_i] = NULL;
}


void lpg_uint_update_from_hex_str(lpg_uint_t *value, const char *hex_str)
{
    affirmf(value,"Expected valid pointer on 'lpg_uint_t' but null was given");
    affirmf(hex_str,"Expected valid pointer on hex string but null was given");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        value_nodes[node_i] = lpg_node_const(value->graph,false);
    
    lpg_uint_assign_from_hex_str(hex_str,value);
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


void lpg_uint_assign_from_hex_str(const char *hex_str, lpg_uint_t *value)
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


#define __lpg_uint_validate_operands_graphs_binary(a,b)                                     \
        affirmf_debug((a)->graph && (b)->graph,"Found operand with no associated graph");   \
        affirmf((a)->graph == (b)->graph,"Operands bounded to different graphs");

#define __lpg_uint_validate_operands_graphs_unary(a)                                        \
        affirmf_debug((a)->graph,"Found operand with no associated graph");


void lpg_uint_copy(lpg_uint_t *dest, lpg_uint_t *src)
{
    affirmf(dest && src,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(dest,src);

    lpg_graph_t *graph = dest->graph;

    lpg_node_t **dest_nodes = lpg_uint_nodes(dest);
    lpg_node_t **src_nodes = lpg_uint_nodes(src);

    size_t upper_bound = MIN(dest->width,src->width);
    size_t node_i = 0;
    for(; node_i < upper_bound; ++node_i)
        dest_nodes[node_i] = src_nodes[node_i];
    
    for(; node_i < dest->width; ++node_i)
        dest_nodes[node_i] = lpg_node_const(graph,false);
}


static inline void __lpg_uint_add_left_smaller(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    lpg_graph_t *graph = a->graph;

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


void lpg_uint_add(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);
    
    if(a->width < b->width)
        __lpg_uint_add_left_smaller(a,b,result);
    else
        __lpg_uint_add_left_smaller(b,a,result);
}


void lpg_uint_add_ip(lpg_uint_t *dest, lpg_uint_t *other)
{
    affirmf(dest && other,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(dest,other);

    lpg_graph_t *graph = dest->graph;

    lpg_node_t **dest_nodes = lpg_uint_nodes(dest);
    lpg_node_t **other_nodes = lpg_uint_nodes(other);

    lpg_node_t *carry = lpg_node_const(graph,false);
    size_t upper_bound = MIN(dest->width,other->width);
    size_t node_i = 0;
    for(; node_i < upper_bound; ++node_i)
    {
        lpg_node_t *terms_part = lpg_node_xor(graph,dest_nodes[node_i],other_nodes[node_i]);
        lpg_node_t *terms_conj = lpg_node_and(graph,dest_nodes[node_i],other_nodes[node_i]);
        dest_nodes[node_i] = lpg_node_xor(graph,terms_part,carry);
        carry = lpg_node_or(graph,
                    lpg_node_and(graph,terms_part,carry),
                    terms_conj
                );
    }

    for(; node_i < dest->width; ++node_i)
    {
        lpg_node_t *saved_dest = dest_nodes[node_i];
        dest_nodes[node_i] = lpg_node_xor(graph,dest_nodes[node_i],carry);
        carry = lpg_node_and(graph,saved_dest,carry);
    }
}


void lpg_uint_sub(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

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


void lpg_uint_and(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    size_t upper_bound = MIN(MIN(a->width,b->width),result->width);
    size_t node_i = 0;
    for(; node_i < upper_bound; ++node_i)
        result_nodes[node_i] = lpg_node_and(graph,a_nodes[node_i],b_nodes[node_i]);
    
    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


void lpg_uint_or(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_uint_t *min_term,*max_term;
    if(a->width < b->width)
    {
        min_term = a;
        max_term = b;
    }
    else
    {
        min_term = b;
        max_term = a;
    }

    lpg_node_t **min_term_nodes = lpg_uint_nodes(min_term);
    lpg_node_t **max_term_nodes = lpg_uint_nodes(max_term);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    size_t min_upper_bound = MIN(min_term->width,result->width);
    size_t node_i = 0;
    for(; node_i < min_upper_bound; ++node_i)
        result_nodes[node_i] = lpg_node_or(graph,min_term_nodes[node_i],max_term_nodes[node_i]);
    
    size_t max_upper_bound = MIN(max_term->width,result->width);
    for(; node_i < max_upper_bound; ++node_i)
        result_nodes[node_i] = max_term_nodes[node_i];
    
    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


void lpg_uint_xor(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_uint_t *min_term,*max_term;
    if(a->width < b->width)
    {
        min_term = a;
        max_term = b;
    }
    else
    {
        min_term = b;
        max_term = a;
    }

    lpg_node_t **min_term_nodes = lpg_uint_nodes(min_term);
    lpg_node_t **max_term_nodes = lpg_uint_nodes(max_term);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    size_t min_upper_bound = MIN(min_term->width,result->width);
    size_t node_i = 0;
    for(; node_i < min_upper_bound; ++node_i)
        result_nodes[node_i] = lpg_node_xor(graph,min_term_nodes[node_i],max_term_nodes[node_i]);
    
    size_t max_upper_bound = MIN(max_term->width,result->width);
    for(; node_i < max_upper_bound; ++node_i)
        result_nodes[node_i] = max_term_nodes[node_i];
    
    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


void lpg_uint_lshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result)
{
    affirmf(a && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_unary(a);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    int64_t node_i = result->width-1;
    for(; node_i >= (int64_t)(a->width+shift); --node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);

    for(; node_i >= (int64_t)shift; --node_i)
        result_nodes[node_i] = a_nodes[node_i-shift];
    
    for(; node_i >= 0; --node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


void lpg_uint_rshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result)
{
    affirmf(a && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_unary(a);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    shift = MIN(a->width,shift);

    size_t node_i = 0;
    size_t upper_bound = MIN(a->width-shift,result->width);
    for(; node_i < upper_bound; ++node_i)
        result_nodes[node_i] = a_nodes[node_i+shift];
    
    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


void lpg_uint_mul(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **b_nodes = lpg_uint_nodes(b);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    for(size_t node_i = 0; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);

    size_t upper_bound = MIN(result->width,b->width);
    lpg_uint_t *a_shifted = lpg_uint_allocate(graph,result->width);
    lpg_uint_t *b_mask = lpg_uint_allocate(graph,result->width);
    lpg_uint_t *a_masked = lpg_uint_allocate(graph,result->width);
    for(size_t node_i = 0; node_i < upper_bound; ++node_i)
    {
        lpg_uint_update_fill_with_single(b_mask,b_nodes[node_i]);
        lpg_uint_lshift(a,node_i,a_shifted);
        lpg_uint_and(a_shifted,b_mask,a_masked);
        if(__likely(node_i > 0))
            lpg_uint_add_ip(result,a_masked);
        else
            lpg_uint_copy(result,a_masked);
    }
    lpg_uint_release(a_shifted);
    lpg_uint_release(b_mask);
    lpg_uint_release(a_masked);
}