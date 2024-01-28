#include <lockpick/graph/types/uint.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <lockpick/math.h>
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

    _uint->__nodes_own = 0;
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


lpg_uint_t *lpg_uint_allocate_as_buffer_view(lpg_graph_t *graph, lpg_node_t **nodes, size_t width)
{
    affirmf(graph,"Expected valid graph pointer but null was given");
    affirmf(nodes,"Expected valid pointer on nodes buffer but null was given");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    __lpg_uint_set_nodes(_uint,nodes);
    __lpg_uint_set_own(_uint,false);

    return _uint;
}


lpg_uint_t *lpg_uint_allocate_as_uint_view(lpg_graph_t *graph, lpg_uint_t *other, size_t offset, size_t width)
{
    affirmf(graph,"Expected valid graph pointer but null was given");
    affirmf(other,"Expected valid pointer on 'lpg_uint_t' object but null was given");
    affirmf(offset <= other->width && (width == LP_NPOS || other->width >= (width+offset)),
        "Can't set view on specified 'lpg_uint_t' with requested offset and width");
    
    if(width == LP_NPOS)
        width = other->width-offset;

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    if(width > 0)
    {
        lpg_node_t **other_nodes = lpg_uint_nodes(other);
        __lpg_uint_set_nodes(_uint,other_nodes+offset);
    }
    else
        __lpg_uint_set_nodes(_uint,NULL);

    __lpg_uint_set_own(_uint,false);

    return _uint;
}


static inline bool __lpg_uint_is_valid_uint(const lpg_uint_t *value)
{
    if(!value)
        return false;
    if(!value->graph)
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
        if(nodes_buff)
            free(nodes_buff);
    }

    free(_uint);
}


void lpg_uint_compute(lpg_uint_t *value)
{
    affirmf(value,"Expected valid pointer on 'lpg_uint_t' but null was given");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        lpg_node_compute(value_nodes[node_i]);
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


static inline void __lpg_uint_add_right_wider(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
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
        __lpg_uint_add_right_wider(a,b,result);
    else
        __lpg_uint_add_right_wider(b,a,result);
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


void lpg_uint_and_ip(lpg_uint_t *dest, lpg_uint_t *other)
{
    affirmf(dest && other,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(dest,other);

    lpg_graph_t *graph = dest->graph;

    lpg_node_t **dest_nodes = lpg_uint_nodes(dest);
    lpg_node_t **other_nodes = lpg_uint_nodes(other);

    size_t node_i = 0;
    size_t upper_bound = MIN(dest->width,other->width);
    for(; node_i < upper_bound; ++node_i)
        dest_nodes[node_i] = lpg_node_and(graph,dest_nodes[node_i],other_nodes[node_i]);
    
    for(; node_i < dest->width; ++node_i)
        dest_nodes[node_i] = lpg_node_const(graph,false);
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


void lpg_uint_or_ip(lpg_uint_t *dest, lpg_uint_t *other)
{
    affirmf(dest && other,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(dest,other);

    lpg_graph_t *graph = dest->graph;

    lpg_node_t **dest_nodes = lpg_uint_nodes(dest);
    lpg_node_t **other_nodes = lpg_uint_nodes(other);

    size_t node_i = 0;
    size_t upper_bound = MIN(dest->width,other->width);
    for(; node_i < upper_bound; ++node_i)
        dest_nodes[node_i] = lpg_node_or(graph,dest_nodes[node_i],other_nodes[node_i]);
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


void lpg_uint_xor_ip(lpg_uint_t *dest, lpg_uint_t *other)
{
    affirmf(dest && other,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(dest,other);

    lpg_graph_t *graph = dest->graph;

    lpg_node_t **dest_nodes = lpg_uint_nodes(dest);
    lpg_node_t **other_nodes = lpg_uint_nodes(other);

    size_t node_i = 0;
    size_t upper_bound = MIN(dest->width,other->width);
    for(; node_i < upper_bound; ++node_i)
        dest_nodes[node_i] = lpg_node_xor(graph,dest_nodes[node_i],other_nodes[node_i]);
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


void __lpg_uint_mul_school(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    lpg_graph_t *graph = a->graph;

    lpg_node_t **b_nodes = lpg_uint_nodes(b);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    for(size_t node_i = 0; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);

    size_t upper_bound = MIN(result->width,b->width);
    lpg_uint_t *a_shifted = lpg_uint_allocate(graph,result->width);
    lpg_uint_t *b_mask = lpg_uint_allocate(graph,result->width);
    for(size_t node_i = 0; node_i < upper_bound; ++node_i)
    {
        lpg_uint_update_fill_with_single(b_mask,b_nodes[node_i]);
        lpg_uint_lshift(a,node_i,a_shifted);
        lpg_uint_and_ip(a_shifted,b_mask);
        if(__likely(node_i > 0))
            lpg_uint_add_ip(result,a_shifted);
        else
            lpg_uint_copy(result,a_shifted);
    }
    lpg_uint_release(a_shifted);
    lpg_uint_release(b_mask);
}


static inline size_t __lpg_uint_mul_ops_width(size_t a_width, size_t b_width)
{
    size_t min_ops_width = MIN(a_width,b_width);
    size_t max_ops_width = MAX(a_width,b_width);
    if(min_ops_width == 0)
        return 0;
    if(min_ops_width == 1)
        return max_ops_width;
    return min_ops_width+max_ops_width;
}

static inline size_t __lpg_uint_add_ops_width(size_t a_width, size_t b_width)
{
    size_t min_ops_width = MIN(a_width,b_width);
    size_t max_ops_width = MAX(a_width,b_width);

    return max_ops_width + !!min_ops_width;
}


void __lpg_uint_mul_karatsuba_left_wider(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    size_t a_tr_width = MIN(result->width,a->width);
    lpg_uint_t *a_tr = lpg_uint_allocate_as_buffer_view(graph,a_nodes,a_tr_width);
    size_t b_tr_width = MIN(result->width,b->width);
    lpg_uint_t *b_tr = lpg_uint_allocate_as_buffer_view(graph,b_nodes,b_tr_width);

    for(size_t node_i = 0; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);

    size_t middle = lp_ceil_div_u(a_tr_width,2);

    size_t a0_width = middle;
    lpg_uint_t *a0 = lpg_uint_allocate_as_uint_view(graph,a_tr,0,a0_width);
    lpg_uint_t *a1 = lpg_uint_allocate_as_uint_view(graph,a_tr,a0_width,LP_NPOS);

    size_t b0_width = MIN(middle,b_tr_width);
    lpg_uint_t *b0 = lpg_uint_allocate_as_uint_view(graph,b_tr,0,b0_width);
    lpg_uint_t *b1 = lpg_uint_allocate_as_uint_view(graph,b_tr,b0_width,LP_NPOS);

    size_t z0_width = __lpg_uint_mul_ops_width(a0->width,b0->width);
    lpg_uint_t *z0 = lpg_uint_allocate(graph,z0_width);
    lpg_uint_mul(a0,b0,z0);

    size_t z2_width = __lpg_uint_mul_ops_width(a1->width,b1->width);
    lpg_uint_t *z2 = lpg_uint_allocate(graph,z2_width);
    lpg_uint_mul(a1,b1,z2);

    size_t a_sum_width = __lpg_uint_add_ops_width(a0->width,a1->width);
    lpg_uint_t *a_sum = lpg_uint_allocate(graph,a_sum_width);
    lpg_uint_add(a0,a1,a_sum);

    size_t b_sum_width = __lpg_uint_add_ops_width(b0->width,b1->width);
    lpg_uint_t *b_sum = lpg_uint_allocate(graph,b_sum_width);
    if(b1->width > 0)
        lpg_uint_add(b0,b1,b_sum);
    else
        lpg_uint_copy(b_sum,b0);

    size_t z1_width = __lpg_uint_mul_ops_width(a_sum_width,b_sum_width);
    lpg_uint_t *p1 = lpg_uint_allocate(graph,z1_width);
    lpg_uint_mul(a_sum,b_sum,p1);

    lpg_uint_t *p2 = lpg_uint_allocate(graph,z1_width);
    if(b1->width > 0)
        lpg_uint_sub(p1,z2,p2);
    else
        lpg_uint_copy(p2,p1);

    lpg_uint_t *z1 = lpg_uint_allocate(graph,z1_width);
    lpg_uint_sub(p2,z0,z1);

    size_t result_v0_width = MIN(z0_width,result->width);
    lpg_uint_t *result_v0 = lpg_uint_allocate_as_uint_view(graph,result,0,result_v0_width);
    size_t result_v1_width = MIN(z1_width,result->width-middle);
    lpg_uint_t *result_v1 = lpg_uint_allocate_as_uint_view(graph,result,middle,result_v1_width);
    size_t result_v2_offset = MIN(z0_width,result->width);
    lpg_uint_t *result_v2 = lpg_uint_allocate_as_uint_view(graph,result,result_v2_offset,LP_NPOS);

    lpg_uint_add_ip(result_v0,z0);
    lpg_uint_add_ip(result_v1,z1);
    lpg_uint_add_ip(result_v2,z2);

    lpg_uint_release(a_tr);
    lpg_uint_release(b_tr);
    lpg_uint_release(a0);
    lpg_uint_release(a1);
    lpg_uint_release(b0);
    lpg_uint_release(b1);
    lpg_uint_release(a_sum);
    lpg_uint_release(b_sum);
    lpg_uint_release(p1);
    lpg_uint_release(p2);
    lpg_uint_release(z0);
    lpg_uint_release(z1);
    lpg_uint_release(z2);
    lpg_uint_release(result_v0);
    lpg_uint_release(result_v1);
    lpg_uint_release(result_v2);
}


void __lpg_uint_mul_karatsuba(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a->width > 1 && b->width > 1,"Can't run Karatsuba multiplication on such narrow numbers.");
    if(a->width > b->width)
        __lpg_uint_mul_karatsuba_left_wider(a,b,result);
    else
        __lpg_uint_mul_karatsuba_left_wider(b,a,result);
}


void lpg_uint_mul(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a && b && result,"Expected valid pointer but null was given");
    __lpg_uint_validate_operands_graphs_binary(a,b);

    size_t width_product = a->width*b->width;
    if(width_product < __LPG_UINT_KARATSUBA_BOUND)
        __lpg_uint_mul_school(a,b,result);
    else
        __lpg_uint_mul_karatsuba(a,b,result);
}