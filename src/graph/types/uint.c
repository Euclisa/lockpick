#include <lockpick/graph/types/uint.h>
#include <lockpick/affirmf.h>
#include <lockpick/define.h>
#include <lockpick/math.h>
#include <string.h>
#include <stdio.h>

#define __LPG_UINT_BAD_CHAR 255

#define __LPG_UINT_NODES_MASK ((uintptr_t)(~0b1))
#define __LPG_UINT_OWN_MASK ((uintptr_t)0b1)


/**
 * lpg_uint_nodes - returns nodes buffer of specified uint
 * @value:      pointer on uint to return nodes buffer for
 * 
 * Return: Nodes buffer of @value
*/
inline lpg_node_t **lpg_uint_nodes(const lpg_uint_t *value)
{
    return (lpg_node_t **)(value->__nodes_own & __LPG_UINT_NODES_MASK);
}

/**
 * __lpg_uint_set_nodes - sets nodes buffer for given uint to specified pointer
 * @value:      pointer on uint to set nodes buffer in
 * @nodes:      nodes buffer
 * 
 * Return: None
*/
static inline void __lpg_uint_set_nodes(lpg_uint_t *value, lpg_node_t **nodes)
{
    value->__nodes_own = (uintptr_t)nodes | (value->__nodes_own & 0b1);
}


/**
 * __lpg_uint_own - returns ownership flag of nodes buffer of specified uint
 * @value:      pointer on uint to return flag on
 * 
 * Ownership flag is stored in the lowest bit of nodes buffer pointer
 * and shows whether it was allocated by this uint object and should be deallocated on its release or outside
 * 
 * Return: Nodes buffer ownership flag.
*/
static inline bool __lpg_uint_own(const lpg_uint_t *value)
{
    return value->__nodes_own & __LPG_UINT_OWN_MASK;
}


/**
 * __lpg_uint_set_own - sets ownership flag of nodes buffer of specified uint
 * @value:      pointer on uint to set flag in
 * @own:        ownership flag value
 * 
 * Ownership flag is stored in the lowest bit of nodes buffer pointer
 * and shows whether it was allocated by this uint object and should be deallocated on its release or outside
 * 
 * Return: None
*/
static inline void __lpg_uint_set_own(lpg_uint_t *value, bool own)
{
    if(own)
        value->__nodes_own |= __LPG_UINT_OWN_MASK;
    else
        value->__nodes_own &= ~__LPG_UINT_OWN_MASK;
}


/**
 * __lpg_uint_general_init - performs allocation and basic initialization common for every type of allocation routines
 * @graph:      graph that the new uint will belong to
 * @width:      width of the uint in bits 
 * 
 * Return: Pointer on allocated uint object with assigned 'graph' and 'width' fields
*/
static inline lpg_uint_t *__lpg_uint_general_init(lpg_graph_t *graph, size_t width)
{
    lpg_uint_t *_uint = (lpg_uint_t*)malloc(sizeof(lpg_uint_t));
    affirmf(_uint,"Failed to allocate space for 'lpg_uint_t' object");

    _uint->__nodes_own = 0;
    _uint->graph = graph;
    _uint->width = width;

    return _uint;
}


/**
 * lpg_uint_allocate - allocate an empty uint  
 * @graph:      graph that the new uint will belong to
 * @width:      width of the uint in bits 
 *
 * This allocates a new uint object with an internal buffer of @width bits.  
 * The buffer is initialized to NULL values.
 *
 * CAUTION: The returned uint should NOT be used as an operand in any graph 
 * assembly operations. Doing so will corrupt the graph. This uint is only meant
 * to be used as a result storage.
 *  
 * Return: Pointer to an initialized uint with empty @width bit buffer
*/
lpg_uint_t *lpg_uint_allocate(lpg_graph_t *graph, size_t width)
{
    affirm_nullptr(graph,"graph");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    lpg_node_t **nodes = (lpg_node_t**)malloc(sizeof(lpg_node_t*)*width);
    affirmf(nodes,"Failed to allocate buffer for nodes");

    for(size_t node_i = 0; node_i < width; ++node_i)
        nodes[node_i] = NULL;

    __lpg_uint_set_nodes(_uint,nodes);
    __lpg_uint_set_own(_uint,true);

    return _uint;
}


/**
 * lpg_uint_allocate_as_buffer_view - allocate a uint viewing a nodes buffer   
 * @graph:      graph that the new uint will belong to
 * @nodes:      buffer of nodes to create a view on
 * @width:      width of the uint in bits  
 *
 * This allocates a new uint object that provides a bit view onto the given @nodes  
 * buffer. The uint does not make a copy of the buffer, but directly references
 * the bits in @nodes for its value.
 *
 * The uint will span @width number of bits starting from the beginning of the 
 * @nodes buffer. Changes made via the view will affect @nodes.
 *
 * The uint does not take ownership of @nodes. Therefore, the buffer will not
 * be freed automatically on view release.
 *  
 * Return: Pointer to a new uint acting as a bit view onto @nodes
*/
lpg_uint_t *lpg_uint_allocate_as_buffer_view(lpg_graph_t *graph, lpg_node_t **nodes, size_t width)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(nodes,"nodes buffer");

    lpg_uint_t *_uint = __lpg_uint_general_init(graph,width);

    __lpg_uint_set_nodes(_uint,nodes);
    __lpg_uint_set_own(_uint,false);

    return _uint;
}


/**
 * lpg_uint_allocate_as_uint_view - allocate a uint as a view on another uint
 * @graph:      graph that the new uint will belong to 
 * @other:      base uint to create a view on 
 * @offset:     bit offset within @other for the start of the view
 * @width:      width of the view in bits. Use LP_NPOS to view from @offset to end of @other
 *
 * This allocates a new uint object that provides a view onto a subset of bits from an existing 
 * uint @other. The view starts at bit offset @offset within @other and spans @width number of  
 * bits. If @width is LP_NPOS, the view goes from @offset to the end of @other.
 *   
 * The returned view does not allocate any additional storage, but references the existing 
 * storage of @other. Any changes made via the view will affect @other as well.
 *
 * Return: Pointer to a new uint acting as a bit view onto @other
*/
lpg_uint_t *lpg_uint_allocate_as_uint_view(lpg_graph_t *graph, lpg_uint_t *other, size_t offset, size_t width)
{
    affirm_nullptr(graph,"graph");
    affirm_nullptr(other,"uint value to set view on");
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


/**
 * lpg_uint_update_from_nodes - updates uint nodes buffer with values from specified nodes buffer
 * @value:      uint object which nodes buffer should be updated
 * @nodes:      buffer of new nodes
 * 
 * This overwrites existing nodes buffer with values inside @nodes. Changes made
 * to updated uint's nodes buffer will not affect @nodes in any way.
 * 
 * CAUTION: All nodes inside @nodes must belong to the same graph @value associated with.
 * 
 * Return: None
*/
void lpg_uint_update_from_nodes(lpg_uint_t *value, lpg_node_t **nodes)
{
    affirm_nullptr(value,"uint value");
    affirm_nullptr(nodes,"nodes buffer");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
    {
        affirmf(__lpg_node_belongs_to_graph(value->graph,nodes[node_i]),
        "Node inside buffer at index '%ld' does not belong to the graph given value bounded with");
        value_nodes[node_i] = nodes[node_i];
    }
}


/**
 * lpg_uint_update_fill_with_single - updates uint nodes buffer assigning all entries to specified node
 * @value:      uint object which nodes buffer should be updated
 * @node:       node object nodes buffer should be filled with
 * 
 * CAUTION: @node must belong to the same graph @value associated with.
 * 
 * Return: None
*/
void lpg_uint_update_fill_with_single(lpg_uint_t *value, lpg_node_t *node)
{
    affirm_nullptr(value,"uint value");
    affirm_nullptr(node,"node");
    affirmf(__lpg_node_belongs_to_graph(value->graph,node),"Specified node does not belong to the graph given value bounded with");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        value_nodes[node_i] = node;
}


/**
 * lpg_uint_update_fill_with_single - updates uint nodes buffer setting all entries to NULL
 * @value:      uint object which nodes buffer should be updated
 * @node:       node object nodes buffer should be filled with
 * 
 * CAUTION: Updated uint should NOT be used as an operand in any graph 
 * assembly operations. Doing so will corrupt the graph. This uint is only meant
 * to be used as a result storage.
 * 
 * Return: None
*/
void lpg_uint_update_empty(lpg_uint_t *value)
{
    affirm_nullptr(value,"uint value");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        value_nodes[node_i] = NULL;
}


/**
 * lpg_uint_update_from_hex_str - update uint buffer from hex string
 * @value:      uint object to update 
 * @hex_str:    hexadecimal string containing new value  
 *  
 * This updates the internal nodes buffer of @value to contain constant  
 * nodes that represent the provided hexadecimal string @hex_str.
 *
 * Nodes are allocated from the graph slab to store the hex string 
 * digits. Existing buffer contents in @value are overwritten.
 * 
 * The string does not need to completely fill the @value width.  
 * Extra MSbits will be set to 0. The string can exceed @value width too.
 *
 * Return: None
*/
void lpg_uint_update_from_hex_str(lpg_uint_t *value, const char *hex_str)
{
    affirm_nullptr(value,"uint value");
    affirm_nullptr(hex_str,"hex-string");

    lpg_node_t **value_nodes = lpg_uint_nodes(value);

    for(size_t node_i = 0; node_i < value->width; ++node_i)
        value_nodes[node_i] = lpg_node_const(value->graph,false);
    
    lpg_uint_assign_from_hex_str(value,hex_str);
}


/**
 * __lpg_uint_update_from_uint - update uint from long arithmetic uint value
 * @value:              uint object to update 
 * @uint_value:         pointer to standard uint value 
 * @uint_value_size:    size of @uint_value in words
 *  
 * This updates the internal nodes buffer of @value to contain 
 * constant nodes representing the long arithmetic uint value provided
 * in @uint_value. 
 *
 * The @uint_value is assumed to be @uint_value_size words long.
 * Existing contents in @value nodes buffer are overwritten.
 *
 * If @uint_value is shorter than @value's width, extra upper bits  
 * are set to 0. If longer, excess words are ignored.
 * 
 * CAUTION: This is not supposed to be called by user.
 * One should use 'lpg_uint_update_from_uint' 
 * that substitutes @uint_value_size automatically.
 *
 * Return: None
*/
void __lpg_uint_update_from_uint(lpg_uint_t *value, const __lp_uint_word_t *uint_value, size_t uint_value_size)
{
    affirm_nullptr(value,"graph uint value");
    affirm_nullptr(uint_value,"uint value");

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


/**
 * lpg_uint_release - releases uint object
 * @_uint:      uint object to be released
 * 
 * Deallocates uint's nodes buffer only if ownership flag is set.
 * 
 * CAUTION: @_uint pointer becomes invalid after this call.
 * 
 * Return: None
*/
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


/**
 * lpg_uint_compute - computes nodes of specified uint object
 * @value:      uint object to compute nodes of
 * 
 * Computes full graph with outputs at @value's nodes buffer.
 * 
 * Return: None
*/
void lpg_uint_compute(lpg_uint_t *value)
{
    affirm_nullptr(value,"uint value");

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
static inline uint8_t __lpg_uint_ch2i(char char_hex)
{
    if(char_hex >= '0' && char_hex <= '9')
        return char_hex - '0';
    if(char_hex >= 'A' && char_hex <= 'F')
        return char_hex - 'A' + 10;
    if(char_hex >= 'a' && char_hex <= 'f')
        return char_hex - 'a' + 10;
    return __LPG_UINT_BAD_CHAR;
}


/**
 * lpg_uint_assign_from_hex_str - assigns uint nodes values from hex string
 * @value:      uint object to change values of
 * @hex_str:    hexadecimal string containing new value
 * 
 * This only sets values of all nodes in @value's nodes buffer according to
 * the given hex string but does not set 'computed' flag.
 * 
 * Return: None
*/
void lpg_uint_assign_from_hex_str(lpg_uint_t *value, const char *hex_str)
{
    affirm_nullptr(value,"uint value");
    affirm_nullptr(hex_str,"hex-string");

    lpg_node_t **nodes = lpg_uint_nodes(value);

    size_t hex_str_len = strlen(hex_str);
    size_t upper_bound = MIN(value->width,hex_str_len*LP_BITS_PER_HEX);
    for(size_t node_i = 0; node_i < upper_bound; node_i += LP_BITS_PER_HEX)
    {
        size_t curr_hex_i = hex_str_len - node_i/LP_BITS_PER_HEX - 1;
        uint8_t curr_hex = __lpg_uint_ch2i(hex_str[curr_hex_i]);
        affirmf(curr_hex != __LPG_UINT_BAD_CHAR,"Unexpected character '%c' inside hex-string",hex_str[curr_hex_i]);

        for(size_t bit_offset = 0; bit_offset < LP_BITS_PER_HEX; ++bit_offset)
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


/**
 * __lpg_uint_i2ch - converts value to hex symbol
 * @value:      integer value to be converted
 * 
 * This converts @value from integer representation
 * to a single hex character. Thus, @value must be within range [0, 16).
 * 
 * Return: Hex character corresponding to @value
*/
static inline char __lpg_uint_i2ch(uint8_t value)
{
    static const char i2ch_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    affirmf_debug(value < sizeof(i2ch_map),"Hex value out of range");

    return i2ch_map[value];
}


/** 
 * lpg_uint_to_hex - Convert uint to hexadecimal string
 * @value: Uint object to convert
 * @dest: Destination buffer for hex string 
 * @n: Size of @dest buffer in bytes
 *
 * This converts the given @value uint into a hexadecimal string
 * representation. The string is written to the buffer @dest,
 * up to its maximum capacity of @n bytes excluding null terminator.
 *
 * The generated string will not contain any leading zeroes or
 * formatting. Bytes are rendered in little-endian order.
 * 
 * If the uint width in bits divided by 4 overflows the destination 
 * buffer, the string is truncated.
 *
 * Return: None 
*/
size_t lpg_uint_to_hex(const lpg_uint_t *value, char *dest, size_t n)
{
    affirm_nullptr(value,"uint value");

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

    size_t hex_str_len = significant_bits_offset/LP_BITS_PER_HEX + 1;
    
    if(dest != NULL)
    {
        size_t hex_str_len_truncated = MIN(n,hex_str_len);
        dest[hex_str_len_truncated] = '\0';

        int64_t curr_bit_i = significant_bits_offset;
        size_t curr_hex_i = 0;

        while(curr_hex_i < hex_str_len_truncated)
        {
            uint8_t curr_hex_value = 0;
            int64_t curr_hex_low_bit_i = (curr_bit_i/LP_BITS_PER_HEX)*LP_BITS_PER_HEX;
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


#define __lpg_uint_validate_operand_graphs_binary(a,b)                                      \
        affirmf_debug((a)->graph && (b)->graph,"Found operand with no associated graph");   \
        affirmf((a)->graph == (b)->graph,"Operands bounded to different graphs");

#define __lpg_uint_validate_operand_graphs_unary(a)                                         \
        affirmf_debug((a)->graph,"Found operand with no associated graph");


/**
 * lpg_uint_copy - Copy uint value between uints
 * @dest: Destination uint object  
 * @src: Source uint object
 *  
 * This copies the value represented by the @src uint into 
 * the @dest uint. 
 *
 * Specifically, it copies the contents of the nodes buffer
 * from @src into the nodes buffer of @dest. 
 *
 * The width of @dest must be greater than or equal to the
 * width of @src to fully capture the value. If @dest is wider,
 * excess upper bits are set to 0.
 * If narrower, exceeding @src nodes are discarded.
 *
 * Return: None
*/
void lpg_uint_copy(lpg_uint_t *dest, lpg_uint_t *src)
{
    affirm_nullptr(dest,"destination uint");
    affirm_nullptr(src,"source uint");
    __lpg_uint_validate_operand_graphs_binary(dest,src);

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


/**
 * __lpg_uint_add_right_wider - uint addition with wider right operand
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store summation  
 * 
 * Performs uint addition between @a and @b, storing the result
 * in @result nodes buffer.
 *
 * @b must be greater than or equal in width to @a. If @b is wider, excess
 * upper bits are carried transparently during addition.
 *
 * The graph assembly for the addition algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * WARNING: No consistency checks are performed. @a, @b, and @result must 
 * belong to the same graph or behavior is undefined.
 *
 * Return: None
*/
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
    else
        lpg_graph_release_node(graph,carry);
    ++node_i;

    for(; node_i < result->width; ++node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


/**
 * lpg_uint_add - uint addition operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store result in  
 * 
 * Performs uint addition between @a and @b, storing the result
 * in @result nodes buffer.
 * 
 * The graph assembly for the addition algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_add(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_binary(a,b);
    
    if(a->width < b->width)
        __lpg_uint_add_right_wider(a,b,result);
    else
        __lpg_uint_add_right_wider(b,a,result);
}


/**
 * lpg_uint_add_ip - inplace uint addition operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * 
 * Performs uint addition between @a and @b, storing the result
 * in @a nodes buffer.
 * 
 * The graph assembly for the addition algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_add_ip(lpg_uint_t *a, lpg_uint_t *b)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);

    lpg_node_t *carry = lpg_node_const(graph,false);
    size_t upper_bound = MIN(a->width,b->width);
    size_t node_i = 0;
    for(; node_i < upper_bound; ++node_i)
    {
        lpg_node_t *terms_part = lpg_node_xor(graph,a_nodes[node_i],b_nodes[node_i]);
        lpg_node_t *terms_conj = lpg_node_and(graph,a_nodes[node_i],b_nodes[node_i]);
        a_nodes[node_i] = lpg_node_xor(graph,terms_part,carry);
        carry = lpg_node_or(graph,
                    lpg_node_and(graph,terms_part,carry),
                    terms_conj
                );
    }

    for(; node_i < a->width; ++node_i)
    {
        lpg_node_t *saved_dest = a_nodes[node_i];
        a_nodes[node_i] = lpg_node_xor(graph,a_nodes[node_i],carry);
        carry = lpg_node_and(graph,saved_dest,carry);
    }
    lpg_graph_release_node(graph,carry);
}


/**
 * lpg_uint_sub - uint subtraction operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store result in  
 * 
 * Performs uint subtraction between @a and @b, storing the result
 * in @result nodes buffer.
 * 
 * The graph assembly for the subtraction algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_sub(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_binary(a,b);

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

    if(node_i == result->width)
        lpg_graph_release_node(graph,carry);
    else
        for(; node_i < result->width; ++node_i)
            result_nodes[node_i] = carry;
}


/**
 * lpg_uint_sub_ip - inplace uint subtraction operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * 
 * Performs uint subtraction between @a and @b, storing the result
 * in @a nodes buffer.
 * 
 * The graph assembly for the subtraction algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_sub_ip(lpg_uint_t *a, lpg_uint_t *b)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);

    lpg_node_t *carry = lpg_node_const(graph,false);
    size_t upper_bound = MIN(a->width,b->width);
    size_t node_i = 0;
    for(; node_i < upper_bound; ++node_i)
    {
        lpg_node_t *terms_part = lpg_node_xor(graph,a_nodes[node_i],b_nodes[node_i]);
        lpg_node_t *carry_conj_part = lpg_node_and(graph,
                                        lpg_node_not(graph,a_nodes[node_i]),
                                        b_nodes[node_i]
                                    );
        a_nodes[node_i] = lpg_node_xor(graph,terms_part,carry);
        carry = lpg_node_or(graph,
                    lpg_node_and(graph,
                        lpg_node_not(graph,terms_part),
                        carry
                    ),
                    carry_conj_part
                );
    }
    
    for(; node_i < a->width; ++node_i)
    {
        lpg_node_t *curr_dest_node = a_nodes[node_i];
        a_nodes[node_i] = lpg_node_xor(graph,a_nodes[node_i],carry);
        carry = lpg_node_and(graph,
                    lpg_node_not(graph,curr_dest_node),
                    carry
                );
    }

    lpg_graph_release_node(graph,carry);
}


/**
 * lpg_uint_and - uint bitwise and operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store result in  
 * 
 * Performs uint bitwise and between @a and @b, storing the result
 * in @result nodes buffer.
 * 
 * The graph assembly for the bitwise and algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_and(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_binary(a,b);

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


/**
 * lpg_uint_and_ip - inplace uint bitwise and operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * 
 * Performs uint bitwise and between @a and @b, storing the result
 * in @a nodes buffer.
 * 
 * @a, @b must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_and_ip(lpg_uint_t *a, lpg_uint_t *b)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);

    size_t node_i = 0;
    size_t upper_bound = MIN(a->width,b->width);
    for(; node_i < upper_bound; ++node_i)
        a_nodes[node_i] = lpg_node_and(graph,a_nodes[node_i],b_nodes[node_i]);
    
    for(; node_i < a->width; ++node_i)
        a_nodes[node_i] = lpg_node_const(graph,false);
}


/**
 * lpg_uint_or - uint bitwise or operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store result in  
 * 
 * Performs uint bitwise or between @a and @b, storing the result
 * in @result nodes buffer.
 * 
 * The graph assembly for the bitwise or algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_or(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_binary(a,b);

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


/**
 * lpg_uint_or_ip - inplace uint bitwise or operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * 
 * Performs uint bitwise or between @a and @b, storing the result
 * in @a nodes buffer.
 * 
 * @a, @b must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_or_ip(lpg_uint_t *a, lpg_uint_t *b)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);

    size_t node_i = 0;
    size_t upper_bound = MIN(a->width,b->width);
    for(; node_i < upper_bound; ++node_i)
        a_nodes[node_i] = lpg_node_or(graph,a_nodes[node_i],b_nodes[node_i]);
}


/**
 * lpg_uint_xor - uint bitwise and operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store result in  
 * 
 * Performs uint bitwise xor between @a and @b, storing the result
 * in @result nodes buffer.
 * 
 * The graph assembly for the bitwise xor algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_xor(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_binary(a,b);

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


/**
 * lpg_uint_xor_ip - inplace uint bitwise xor operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * 
 * Performs uint bitwise xor between @a and @b, storing the result
 * in @a nodes buffer.
 * 
 * @a, @b must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_xor_ip(lpg_uint_t *a, lpg_uint_t *b)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **b_nodes = lpg_uint_nodes(b);

    size_t node_i = 0;
    size_t upper_bound = MIN(a->width,b->width);
    for(; node_i < upper_bound; ++node_i)
        a_nodes[node_i] = lpg_node_xor(graph,a_nodes[node_i],b_nodes[node_i]);
}


/**
 * lpg_uint_lshift - left shift uint by bits 
 * @a:          uint operand to shift
 * @shift:      number of bits to shift left by 
 * @result:     uint object to store result
 * 
 * This left shifts the value in @a by @shift number of bits, 
 * storing the result in @result nodes buffer.
 *
 * Excess bits shifted off the left edge are discarded.
 * Zeroes are shifted in from the right side.
 *
 * If @shift exceeds the width of @a, @result will be fully zeroed out. @a 
 * width sets the maximum useful shift distance.
 *
 * Return: None
*/
void lpg_uint_lshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result)
{
    affirm_nullptr(a,"uint operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_unary(a);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);
    lpg_node_t **result_nodes = lpg_uint_nodes(result);

    shift = MIN(shift,result->width);
    int64_t node_i = result->width-1;
    for(; node_i >= (int64_t)(a->width+shift); --node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);

    for(; node_i >= (int64_t)shift; --node_i)
        result_nodes[node_i] = a_nodes[node_i-shift];
    
    for(; node_i >= 0; --node_i)
        result_nodes[node_i] = lpg_node_const(graph,false);
}


/**
 * lpg_uint_lshift_ip - inplace left shift uint by bits 
 * @a:          uint operand to shift
 * @shift:      number of bits to shift left by 
 * 
 * This left shifts the value in @a by @shift number of bits, 
 * storing the result in @a nodes buffer.
 *
 * Excess bits shifted off the left edge are discarded.
 * Zeroes are shifted in from the right side.
 *
 * If @shift exceeds the width of @a, result will be fully zeroed out. @a 
 * width sets the maximum useful shift distance.
 *
 * Return: None
*/
void lpg_uint_lshift_ip(lpg_uint_t *a, size_t shift)
{
    affirm_nullptr(a,"uint operand");

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);

    shift = MIN(shift,a->width);
    int64_t node_i = a->width-1;
    for(; node_i >= (int64_t)shift; --node_i)
        a_nodes[node_i] = a_nodes[node_i-shift];
    
    for(; node_i >= 0; --node_i)
        a_nodes[node_i] = lpg_node_const(graph,false);
}


/**
 * lpg_uint_rshift - right shift uint by bits 
 * @a:          uint operand to shift
 * @shift:      number of bits to shift right by 
 * @result:     uint object to store result
 * 
 * This right shifts the value in @a by @shift number of bits, 
 * storing the result in @result nodes buffer.
 *
 * If @shift exceeds the width of @a, @result will be fully zeroed out. @a 
 * width sets the maximum useful shift distance.
 *
 * Return: None
*/
void lpg_uint_rshift(lpg_uint_t *a, size_t shift, lpg_uint_t *result)
{
    affirm_nullptr(a,"uint operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_unary(a);

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


void lpg_uint_rshift_ip(lpg_uint_t *a, size_t shift)
{
    affirm_nullptr(a,"uint operand");
    __lpg_uint_validate_operand_graphs_unary(a);

    lpg_graph_t *graph = a->graph;

    lpg_node_t **a_nodes = lpg_uint_nodes(a);

    shift = MIN(a->width,shift);
    size_t node_i = 0;
    for(; node_i < a->width-shift; ++node_i)
        a_nodes[node_i] = a_nodes[node_i+shift];
    
    for(; node_i < a->width; ++node_i)
        a_nodes[node_i] = lpg_node_const(graph,false);
}


/**
 * __lpg_uint_mul_school - uint multiplication operation using school algorithm
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store summation  
 * 
 * Performs uint multiplication between @a and @b using regular school algorithm,
 * storing the result in @result nodes buffer.
 * 
 * The graph assembly for the multiplication algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void __lpg_uint_mul_school(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    lpg_graph_t *graph = a->graph;

    lpg_node_t **b_nodes = lpg_uint_nodes(b);

    lpg_uint_update_from_hex_str(result,"0");

    size_t upper_bound = MIN(result->width,b->width);
    lpg_uint_t *a_shifted = lpg_uint_allocate(graph,result->width);
    lpg_uint_t *b_mask = lpg_uint_allocate(graph,result->width);
    for(size_t node_i = 0; node_i < upper_bound; ++node_i)
    {
        lpg_uint_update_fill_with_single(b_mask,b_nodes[node_i]);
        lpg_uint_lshift(a,node_i,a_shifted);
        lpg_uint_and_ip(a_shifted,b_mask);
        lpg_uint_add_ip(result,a_shifted);
    }
    lpg_uint_release(a_shifted);
    lpg_uint_release(b_mask);
}


/**
 * __lpg_uint_mul_ops_width - get required width for multiplication result
 * @a_width:    width of left-side operand in bits  
 * @b_width:    width of right-side operand in bits
 *
 * Returns the minimum result uint width needed to store the full 
 * product of two unsigned integers with widths @a_width and @b_width,  
 * without any overflow.
 *
 * This facilitates pre-allocating a properly sized uint to fit  
 * the multiplication output prior to calling the multiply routine.
 * 
 * Return: Minimum uint width to store full multiplication output 
*/
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

/**
 * __lpg_uint_add_ops_width - get required width for addition result
 * @a_width:    width of left-side operand in bits  
 * @b_width:    width of right-side operand in bits
 *
 * Returns the minimum result uint width needed to store the full 
 * sum of two unsigned integers with widths @a_width and @b_width,  
 * without any overflow.
 *
 * This facilitates pre-allocating a properly sized uint to fit  
 * the addition output prior to calling the addition routine.
 * 
 * Return: Minimum uint width to store full addition output 
*/
static inline size_t __lpg_uint_add_ops_width(size_t a_width, size_t b_width)
{
    size_t min_ops_width = MIN(a_width,b_width);
    size_t max_ops_width = MAX(a_width,b_width);

    return max_ops_width + !!min_ops_width;
}


/**
 * __lpg_uint_mul_karatsuba_left_wider - uint multiplication operation using karatsuba algorithm with wider left operand
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store summation  
 * 
 * Performs uint multiplication between @a and @b using karatsuba algorithm,
 * storing the result in @result nodes buffer.
 * 
 * The graph assembly for the multiplication algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a must be greater than or equal in width to @b.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void __lpg_uint_mul_karatsuba_left_wider(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    lpg_graph_t *graph = a->graph;

    size_t a_tr_width = MIN(result->width,a->width);
    lpg_uint_t *a_tr = lpg_uint_allocate_as_uint_view(graph,a,0,a_tr_width);
    size_t b_tr_width = MIN(result->width,b->width);
    lpg_uint_t *b_tr = lpg_uint_allocate_as_uint_view(graph,b,0,b_tr_width);

    lpg_uint_update_from_hex_str(result,"0");

    /*
        'a' always has equal or higher width than 'b',
        that is why we implicitly pad 'b' to 'a' width
    */
    size_t work_width = a_tr_width;

    /*
        Use ceil because lower halve must be wider than higher
    */
    size_t halve_width = lp_ceil_div_u(work_width,2);

    /*
        a = a1 * 2^halve_width + a0
    */
    size_t a0_width = halve_width;
    lpg_uint_t *a0 = lpg_uint_allocate_as_uint_view(graph,a_tr,0,a0_width);
    lpg_uint_t *a1 = lpg_uint_allocate_as_uint_view(graph,a_tr,a0_width,LP_NPOS);

    /*
        b = b1 * 2^halve_width + b0,

            where b1 might have zero width
    */
    size_t b0_width = MIN(halve_width,b_tr_width);
    lpg_uint_t *b0 = lpg_uint_allocate_as_uint_view(graph,b_tr,0,b0_width);
    lpg_uint_t *b1 = lpg_uint_allocate_as_uint_view(graph,b_tr,b0_width,LP_NPOS);

    /*
        z0 = a0 * b0
    */
    size_t z0_width = MIN(result->width,__lpg_uint_mul_ops_width(a0->width,b0->width)); // Maybe don't need MIN here
    lpg_uint_t *z0 = lpg_uint_allocate(graph,z0_width);
    lpg_uint_mul(a0,b0,z0);

    size_t z1_width = MIN(result->width-halve_width,
            __lpg_uint_add_ops_width(__lpg_uint_mul_ops_width(a0->width,b1->width),__lpg_uint_mul_ops_width(a1->width,b0->width)));

    /*
        z2 = a1 * b1
    */
    size_t z2_width = MIN(z1_width,__lpg_uint_mul_ops_width(a1->width,b1->width));
    lpg_uint_t *z2 = lpg_uint_allocate(graph,z2_width);
    lpg_uint_mul(a1,b1,z2);

    /*
        z1 = (a0 + a1)*(b0 + b1) - z0 - z2
    */
    size_t a_sum_width = MIN(z1_width,__lpg_uint_add_ops_width(a0->width,a1->width));
    lpg_uint_t *a_sum = lpg_uint_allocate(graph,a_sum_width);
    lpg_uint_add(a0,a1,a_sum);

    size_t b_sum_width = MIN(z1_width,__lpg_uint_add_ops_width(b0->width,b1->width));
    lpg_uint_t *b_sum = lpg_uint_allocate(graph,b_sum_width);
    if(b1->width > 0)
        lpg_uint_add(b0,b1,b_sum);
    else
        lpg_uint_copy(b_sum,b0);

    lpg_uint_t *z1 = lpg_uint_allocate(graph,z1_width);
    lpg_uint_mul(a_sum,b_sum,z1);

    lpg_uint_sub_ip(z1,z2);
    lpg_uint_sub_ip(z1,z0);

    lpg_uint_t *result_v0 = lpg_uint_allocate_as_uint_view(graph,result,0,LP_NPOS);
    lpg_uint_t *result_v1 = lpg_uint_allocate_as_uint_view(graph,result,halve_width,LP_NPOS);
    lpg_uint_t *result_v2 = lpg_uint_allocate_as_uint_view(graph,result,MIN(result->width,halve_width*2),LP_NPOS);

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
    lpg_uint_release(z0);
    lpg_uint_release(z1);
    lpg_uint_release(z2);
    lpg_uint_release(result_v0);
    lpg_uint_release(result_v1);
    lpg_uint_release(result_v2);
}


/**
 * __lpg_uint_mul_karatsuba - uint multiplication operation using karatsuba algorithm
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store summation  
 * 
 * Performs uint multiplication between @a and @b using karatsuba algorithm,
 * storing the result in @result nodes buffer.
 * 
 * The graph assembly for the multiplication algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void __lpg_uint_mul_karatsuba(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirmf(a->width > 1 && b->width > 1,"Can't run Karatsuba multiplication on such narrow numbers.");
    if(a->width > b->width)
        __lpg_uint_mul_karatsuba_left_wider(a,b,result);
    else
        __lpg_uint_mul_karatsuba_left_wider(b,a,result);
}


/**
 * __lpg_uint_is_mul_karatsuba - check if Karatsuba multiplication algorithm should be used
 * 
 * This function checks if the given operand widths are large enough that 
 * using the Karatsuba multiplication algorithm would be faster than the 
 * standard multiplication algorithm.
 *
 * The Karatsuba algorithm has lower computational complexity for very 
 * large operands. This function determines if the performance crossover
 * point has been reached where Karatsuba would be faster.
 * 
 * @a_width: The bit width of the first unsigned integer operand
 * @b_width: The bit width of the second unsigned integer operand 
 * @result_width: The bit width of the result
 *
 * Return:
 *  - TRUE if the Karatsuba algorithm should be used because the operand
 *    widths meet the minimum threshold
 *  - FALSE if standard multiplication should be used  
 */
inline bool __lpg_uint_is_mul_karatsuba(size_t a_width, size_t b_width, size_t result_width)
{
    size_t width_product = a_width*b_width;
    return width_product < __LPG_UINT_KARATSUBA_BOUND || result_width < 4;
}


/**
 * lpg_uint_mul - uint multiplication operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * @result:     uint object to store summation  
 * 
 * Performs uint multiplication between @a and @b, storing the result
 * in @result nodes buffer.
 * 
 * The graph assembly for the multiplication algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * This chooses multiplication algorithm according to '__lpg_uint_is_mul_karatsuba'
 * predicate. If predicate is 'true' then multiplication is performed using Karatsuba
 * algorithm and regular school algorithm otherwise.
 * 
 * @a, @b, and @result must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_mul(lpg_uint_t *a, lpg_uint_t *b, lpg_uint_t *result)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    affirm_nullptr(result,"result");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    if(__lpg_uint_is_mul_karatsuba(a->width,b->width,result->width))
        __lpg_uint_mul_school(a,b,result);
    else
        __lpg_uint_mul_karatsuba(a,b,result);
}


/**
 * lpg_uint_mul_ip - inplace uint multiplication operation
 * @a:          left-side uint operand  
 * @b:          right-side uint operand
 * 
 * Performs uint multiplication between @a and @b, storing the result
 * in @a nodes buffer.
 * 
 * The graph assembly for the multiplication algorithm may allocate constant nodes.
 * The caller is responsible to optimize them in any moment after operation.
 * 
 * @a, @b must belong to the same graph.
 *
 * Return: None
*/
void lpg_uint_mul_ip(lpg_uint_t *a, lpg_uint_t *b)
{
    affirm_nullptr(a,"left-side operand");
    affirm_nullptr(b,"right-side operand");
    __lpg_uint_validate_operand_graphs_binary(a,b);

    lpg_graph_t *graph = a->graph;

    lpg_uint_t *result = lpg_uint_allocate(graph,a->width);

    lpg_uint_mul(a,b,result);
    lpg_uint_copy(a,result);

    lpg_uint_release(result);
}