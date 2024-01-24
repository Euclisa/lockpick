#include <lockpick/uint.h>
#include <string.h>
#include <stdlib.h>

#define __LP_UINT_BAD_CHAR 255

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

const __uint128_t __LP_UINT_BASE = ((__uint128_t)1 << (__LP_UINT_BITS_PER_WORD));
const __lp_uint_word_t __LP_UINT_MAX_WORD = (__lp_uint_word_t)-1;
const uint64_t __LP_UINT_HEXES_PER_WORD = __LP_UINT_BITS_PER_WORD / __LP_UINT_BITS_PER_HEX;


/**
 * __lp_uint_ch2i - converts hex characters to unsigned integer
 * @char_hex:	hex character ([0-9a-fA-F])
 * 
 * Returns unsigned integer corresponding to the given character.
 * __LP_UINT_BAD_CHAR if 'char_hex' is invalid.
 */
uint8_t __lp_uint_ch2i(char char_hex)
{
    if(char_hex >= '0' && char_hex <= '9')
        return char_hex - '0';
    if(char_hex >= 'A' && char_hex <= 'F')
        return char_hex - 'A' + 10;
    if(char_hex >= 'a' && char_hex <= 'f')
        return char_hex - 'a' + 10;
    return_set_errno(__LP_UINT_BAD_CHAR,EINVAL);
}


/**
 * __lp_uint_parse_hex_word_reverse - parses up to one word in reverse order starting at given position
 * @hex_str:	string containing hex number
 * @start:      index of the character to start from
 * @result:     pointer on a word where to put the result
 * 
 * Returns number of character parsed.
 * -1 if invalid character was found.
 */
int8_t __lp_uint_parse_hex_word_reverse(const char *hex_str, uint32_t start, __lp_uint_word_t *result)
{
    *result = 0;
    int64_t curr_char_i = start;
    
    // Characters are parsed in reverse order so low hexes come first
    for (uint8_t offset = 0; offset < __LP_UINT_BITS_PER_WORD; offset += __LP_UINT_BITS_PER_HEX)
    {
        // If nothing left to parse
        if (curr_char_i < 0)
            break;

        char curr_char = hex_str[curr_char_i];
        __lp_uint_word_t char_converted = __lp_uint_ch2i(curr_char);
        if(char_converted == __LP_UINT_BAD_CHAR)
            return_set_errno(-1,EINVAL);

        *result += char_converted << offset;
        --curr_char_i;
    }

    return start - curr_char_i;
}


/**
 * __lp_uint_from_hex - initializes uint object from hex string
 * @hex_str:	string containing hex number
 * @value:      pointer on uint buffer
 * @value_size: size of uint buffer
 * 
 * Returns status of initialization.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_from_hex' macro instead.
 */
bool __lp_uint_from_hex(const char *hex_str, __lp_uint_word_t *value, size_t value_size)
{
    if(!hex_str || !value)
        return_set_errno(false,EINVAL);

    size_t hex_str_len = strlen(hex_str);
    int64_t curr_char_i = hex_str_len-1;
    size_t curr_word_i = 0;
    while(curr_word_i < value_size && curr_char_i >= 0)
    {
        // Parse up to one word and write right away
        int8_t read_hexes = __lp_uint_parse_hex_word_reverse(hex_str, curr_char_i, value+curr_word_i);
        if(read_hexes < 0)
            return_set_errno(false,EINVAL);
        curr_char_i -= read_hexes;
        ++curr_word_i;
    }

    for(; curr_word_i < value_size; value[curr_word_i++] = 0);

    return true;
}


/**
 * __lp_uint_i2ch - converts single word to hex string
 * @value:          word to convert
 * @dest:           destination pointer on string to store result in
 * @n:              maximum number of hexes to write into dest
 * @truncate_zeros: whether to truncate high zeros or not
 * 
 * Returns number of characters written.
 */
uint8_t __lp_uint_i2ch(__lp_uint_word_t value, char *dest, size_t n, bool truncate_zeros)
{
    static const char i2ch_map[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    // If high order zeros truncation requested then truncate and remember number of truncated
    uint8_t hexes_truncated = 0;
    while(truncate_zeros)
    {
        uint8_t shift = __LP_UINT_BITS_PER_WORD-__LP_UINT_BITS_PER_HEX*(hexes_truncated+1);
        __lp_uint_word_t shifted = value >> shift;
        if(shifted != 0)
            break;
        ++hexes_truncated;
    }

    uint8_t max_hexes_to_write = MIN(__LP_UINT_HEXES_PER_WORD-hexes_truncated,n);
    uint8_t wrote_hexes = 0;
    for(; wrote_hexes < max_hexes_to_write; ++wrote_hexes)
    {
        uint8_t shift = __LP_UINT_BITS_PER_WORD-__LP_UINT_BITS_PER_HEX*(wrote_hexes+hexes_truncated+1);
        __lp_uint_word_t shifted = value >> shift;
        char shifted_hex = i2ch_map[shifted & 0xf];
        dest[wrote_hexes] = shifted_hex;
    }

    return wrote_hexes;
}


/**
 * __lp_uint_to_hex - converts uint object to hex string
 * @value:      pointer on uint buffer
 * @value_size: size of uint buffer
 * @dest:           destination pointer on string to store result in
 * @n:              maximum number of hexes to write into dest
 * 
 * Returns number of hexes in hex string corresponding to 'value'.
 * Negative value is returned on error.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_to_hex' macro instead.
 */
int64_t __lp_uint_to_hex(const __lp_uint_word_t *value, size_t value_size, char *dest, size_t n)
{
    if(!value)
        return_set_errno(-1,EINVAL);

    // Find highest non-zero word
    int64_t significant_words_offset = value_size-1;
    for(; significant_words_offset >= 0 && value[significant_words_offset] == 0; --significant_words_offset);

    // If all words == 0
    if(significant_words_offset < 0)
    {
        if(n > 0 && dest)
        {
            dest[0] = '0';
            dest[1] = '\0';
        }

        return 2;
    }

    // All high zeros in the highest word must be truncated
    size_t hex_str_len = (significant_words_offset) * __LP_UINT_HEXES_PER_WORD;
    for(uint8_t shift = 0; shift < __LP_UINT_BITS_PER_WORD && (value[significant_words_offset] >> shift); shift += __LP_UINT_BITS_PER_HEX)
        ++hex_str_len;
    
    if(dest != NULL)
    {
        size_t hex_str_len_truncated = MIN(n,hex_str_len);
        dest[hex_str_len_truncated] = '\0';

        size_t wrote_hexes = 0;
        wrote_hexes += __lp_uint_i2ch(value[significant_words_offset--],dest+wrote_hexes,hex_str_len_truncated-wrote_hexes,true);
        while(significant_words_offset >= 0)
            wrote_hexes += __lp_uint_i2ch(value[significant_words_offset--],dest+wrote_hexes,hex_str_len_truncated-wrote_hexes,false);
    }
    
    return hex_str_len;
}


/**
 * __lp_uint_copy - copies value from 'src' to 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @src:        pointer on source uint buffer
 * @src_size:   size of source uint buffer
 * 
 * Returns true on success, false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_copy' macro instead.
 */
inline bool __lp_uint_copy(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *src, size_t src_size)
{
    if(dest == NULL || src == NULL)
        return_set_errno(false,EINVAL);
    
    size_t src_upper_bound = MIN(dest_size,src_size);
    size_t word_i = 0;
    for(; word_i < src_upper_bound; ++word_i)
        dest[word_i] = src[word_i];
    
    for(; word_i < dest_size; ++word_i)
        dest[word_i] = 0;
    
    return true;
}


/**
 * __lp_uint_add_2w_inplace - performs addition of 'dest' and 'other' treating other as a uint of size 2 words storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform addition with
 * 
 * Returns nothing because does not perform any checks.
 * 
 * Calls must be performed from higher level functions that check for pointers and sizes validity.
 */
static inline void __lp_uint_add_2w_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other)
{
    __lp_uint_word_t other_w0 = other[0];
    __lp_uint_word_t other_w1 = other[1];
    dest[0] += other_w0;
    __lp_uint_word_t carry = (dest[0] < other_w0) ? 1 : 0;

    if(dest_size > 1)
    {
        __uint128_t sum1 = (__uint128_t)dest[1] + (__uint128_t)other_w1 + (__uint128_t)carry;
        dest[1] = sum1;
        carry = sum1 >> __LP_UINT_BITS_PER_WORD;
    }

    if(carry && dest_size > 2)
    {
        for(size_t word_i = 2; word_i < dest_size; ++word_i)
            if(++dest[word_i] != 0)
                break;
    }
}


/**
 * __lp_uint_add_left_smaller - performs addition treating left side ('a') as a uint of smaller or equal size than right side ('b') storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer; <= @b_size
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer; >= @a_size
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns nothing because does not perform any checks.
 * 
 * Calls must be performed from higher level functions that check for pointers and sizes validity.
 */
static inline void __lp_uint_add_left_smaller(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    __uint128_t carry = 0;
    size_t word_i = 0;
    size_t a_upper_bound = MIN(a_size,result_size);
    for(; word_i < a_upper_bound; ++word_i)
    {
        __uint128_t curr_sum = (__uint128_t)a[word_i] + (__uint128_t)b[word_i] + carry;
        result[word_i] = curr_sum & __LP_UINT_MAX_WORD;
        carry = curr_sum >> __LP_UINT_BITS_PER_WORD;
    }

    size_t b_upper_bound = MIN(b_size,result_size);
    for(; word_i < b_upper_bound; ++word_i)
    {
        __uint128_t curr_sum = (__uint128_t)b[word_i] + carry;
        result[word_i] = curr_sum & __LP_UINT_MAX_WORD;
        carry = curr_sum >> __LP_UINT_BITS_PER_WORD;
    }
    
    if(result_size != b_upper_bound)
    {
        result[b_upper_bound] = carry;
        for(size_t word_i = b_upper_bound+1; word_i < result_size; ++word_i)
            result[word_i] = 0;
    }
}


/**
 * __lp_uint_add - performs addition of 'a' and 'b' storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_add' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a' or 'b'.
 */
inline bool __lp_uint_add(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return_set_errno(false,EINVAL);
    if(a == result || b == result)
        return_set_errno(false,EINVAL);

    if(a_size < b_size)
        __lp_uint_add_left_smaller(a,a_size,b,b_size,result,result_size);
    else
        __lp_uint_add_left_smaller(b,b_size,a,a_size,result,result_size);
    
    return true;
}


/**
 * __lp_uint_add_inplace_bigger - performs addition of 'dest' and 'other' storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform addition with
 * @other_size: size of another uint buffer
 * 
 * Returns true on success, false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_add_ip' macro instead.
 */
inline bool __lp_uint_add_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other, size_t other_size)
{
    if(!dest || !other)
        return_set_errno(false,EINVAL);

    __uint128_t carry = 0;
    size_t word_i = 0;
    size_t other_upper_bound = MIN(other_size,dest_size);
    for(; word_i < other_upper_bound; ++word_i)
    {
        __uint128_t curr_sum = (__uint128_t)dest[word_i] + (__uint128_t)other[word_i] + carry;
        dest[word_i] = curr_sum & __LP_UINT_MAX_WORD;
        carry = curr_sum >> __LP_UINT_BITS_PER_WORD;
    }

    for(; (word_i < dest_size) && (carry > 0); ++word_i)
    {
        __uint128_t curr_sum = (__uint128_t)dest[word_i] + carry;
        dest[word_i] = curr_sum & __LP_UINT_MAX_WORD;
        carry = curr_sum >> __LP_UINT_BITS_PER_WORD;
    }

    return true;
}


/**
 * __lp_uint_sub - performs subtraction of 'a' and 'b' storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_sub' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a' or 'b'.
 */
inline bool __lp_uint_sub(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return_set_errno(false,EINVAL);
    if(a == result || b == result)
        return_set_errno(false,EINVAL);

    __uint128_t carry = 0;
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
        __uint128_t total_neg = b[word_i] + carry;
        if(a[word_i] < total_neg)
        {
            result[word_i] += __LP_UINT_BASE - total_neg;
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
            result[word_i] = __LP_UINT_BASE - (b[word_i] + carry);  // Either carry (then first part was skipped) or bk is not zero now
            carry = 1;
            ++word_i;
            for(; word_i < b_upper_bound; ++word_i)
                result[word_i] = __LP_UINT_BASE - (b[word_i] + carry); // carry and __LP_UINT_BASE are optimized
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
            result[word_i] = __LP_UINT_BASE - 1;
        
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
        result[word_i] = __LP_UINT_BASE - carry;

    return true;
}


/**
 * __lp_uint_sub_inplace - performs subtraction of 'other' from 'dest' storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform addition with
 * @other_size: size of another uint buffer
 * 
 * Returns true on success, false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_sub_ip' macro instead.
 */
inline bool __lp_uint_sub_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other, size_t other_size)
{
    if(!dest || !other)
        return_set_errno(false,EINVAL);

    __uint128_t carry = 0;
    size_t word_i = 0;
    size_t other_upper_bound = MIN(other_size,dest_size);
    for(; word_i < other_upper_bound; ++word_i)
    {
        __uint128_t total_neg = other[word_i] + carry;
        if(dest[word_i] < total_neg)
        {
            dest[word_i] += __LP_UINT_BASE - total_neg;
            carry = 1;
        }
        else
        {
            dest[word_i] -= total_neg;
            carry = 0;
        }
    }

    if(carry > 0)
    {
        for(; word_i < dest_size; ++word_i)
        {
            if(dest[word_i]-- != 0)
                break;
        }
    }

    return true;
}


/**
 * __lp_uint_mul - performs school multiplication of 'a' and 'b' storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_mul' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a' or 'b'.
 */
inline bool __lp_uint_mul(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return_set_errno(false,EINVAL);
    if(a == result || b == result)
        return_set_errno(false,EINVAL);

    for(size_t res_i = 0; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    size_t a_last_i = a_size - 1;
    size_t b_last_i = b_size - 1;
    size_t effective_res_last_i = a_last_i+b_last_i;
    size_t result_upper_bound = MIN(effective_res_last_i,result_size-1);
    size_t res_i = 0;
    for(; res_i <= result_upper_bound; ++res_i)
    {
        size_t a_lower_bound = res_i > b_last_i ? res_i - b_last_i : 0;
        size_t a_upper_bound = MIN(a_last_i,res_i);
        __uint128_t curr_mul;
        for(size_t a_i = a_lower_bound; a_i <= a_upper_bound; ++a_i)
        {
            size_t b_i = res_i-a_i;
            curr_mul = (__uint128_t)a[a_i]*(__uint128_t)b[b_i];
            // Split 128-bit word into two 64-bit word and treat it like a regular 2-word sized uint adding it to corresponding result with offset
            __lp_uint_word_t curr_mul_words[2] = {curr_mul, curr_mul >> __LP_UINT_BITS_PER_WORD};
            __lp_uint_add_2w_inplace(result+res_i,result_size-res_i,curr_mul_words);
        }
    }

    return true;
}


/**
 * __lp_uint_mul_inplace - performs multiplication of 'dest' and 'other' storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform addition with
 * @other_size: size of another uint buffer
 * 
 * Returns true on success, false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_mul_ip' macro instead.
 * 
 * CAUTION: It is preferable to use regular version of multiplication
 * because this inplace version does the same amount of operations
 * plus copying from temporary buffer to 'dest' (plus 'malloc' and 'free' calls).
 */
inline bool __lp_uint_mul_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other, size_t other_size)
{
    // Basically, rewritting procedure above taking into account that 'a_size' ('dest_size') is now equal to 'result_size'
    if(!dest || !other)
        return_set_errno(false,EINVAL);
    
    __lp_uint_word_t *result = (__lp_uint_word_t*)malloc(sizeof(__lp_uint_word_t)*dest_size);
    size_t result_size = dest_size;

    for(size_t res_i = 0; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    size_t other_last_i = other_size - 1;
    size_t res_i = 0;
    for(; res_i < result_size; ++res_i)
    {
        size_t a_lower_bound = res_i > other_last_i ? res_i - other_last_i : 0;
        size_t a_upper_bound = res_i;
        __uint128_t curr_mul;
        for(size_t a_i = a_lower_bound; a_i <= a_upper_bound; ++a_i)
        {
            size_t b_i = res_i-a_i;
            curr_mul = (__uint128_t)dest[a_i]*(__uint128_t)other[b_i];
            // Split 128-bit word into two 64-bit word and treat it like a regular 2-word sized uint adding it to corresponding result with offset
            __lp_uint_word_t curr_mul_words[2] = {curr_mul, curr_mul >> __LP_UINT_BITS_PER_WORD};
            __lp_uint_add_2w_inplace(result+res_i,result_size-res_i,curr_mul_words);
        }
    }

    for(size_t dest_i = 0; dest_i < dest_size; ++dest_i)
        dest[dest_i] = result[dest_i];
    
    free(result);

    return true;
}


/**
 * __lp_uint_eq_left_smaller - performs equality check of 'a' and 'b' treating left side ('a') as a uint of smaller or equal size than right side ('b')
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer; <= 'b_size'
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer; >= 'a_size'
 * 
 * Returns true if 'a' and 'b' represent equal numbers.
 * 
 * Calls must be performed from higher level functions that check for pointers and sizes validity.
 */
static inline bool __lp_uint_eq_left_smaller(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    size_t word_i = 0;
    for(; word_i < a_size; ++word_i)
    {
        if(a[word_i] != b[word_i])
            return false;
    }

    for(; word_i < b_size; ++word_i)
    {
        if(b[word_i] != 0)
            return false;
    }

    return true;
}


/**
 * __lp_uint_eq - performs equality check of 'a' and 'b'
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer
 * 
 * Returns true if 'a' and 'b' represent equal numbers.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_eq' macro instead.
 */
inline bool __lp_uint_eq(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    if(!a || !b)
        return_set_errno(false,EINVAL);
    
    if(a_size < b_size)
        return __lp_uint_eq_left_smaller(a,a_size,b,b_size);
    else
        return __lp_uint_eq_left_smaller(b,b_size,a,a_size);
}


/**
 * __lp_uint_3way - performs three-way comparison of 'a' and 'b'
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer
 * 
 * Returns value of type 'lp_uint_3way_t':
 * LP_UINT_EQUAL - if 'a' and 'b' are equal
 * LP_UINT_LESS - if 'a' is less than 'b'
 * LP_UINT_GREATER - if 'a' is greater than 'b'
 * 
 * This is not supposed to be called by user. Use 'lp_uint_3way' macro instead.
 */
inline lp_uint_3way_t __lp_uint_3way(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    if(!a || !b)
        return_set_errno(false,EINVAL);
    
    const __lp_uint_word_t *max_term;
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
        {
            if(a != max_term)
                return LP_UINT_LESS;
            else
                return LP_UINT_GREATER;
        }
    }

    for(int64_t word_i = min_term_size-1; word_i >= 0; --word_i)
    {
        if(a[word_i] == b[word_i])
            continue;
        if(a[word_i] < b[word_i])
            return LP_UINT_LESS;
        else
            return LP_UINT_GREATER;
    }

    return LP_UINT_EQUAL;
}


/**
 * __lp_uint_ls - performs check if 'a' is less than 'b'
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer
 * 
 * Returns true if 'a' represents number which is less than number represented by 'b'
 * 
 * This is not supposed to be called by user. Use 'lp_uint_ls' macro instead.
 */
inline bool __lp_uint_ls(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    return __lp_uint_3way(a,a_size,b,b_size) == LP_UINT_LESS;
}


/**
 * __lp_uint_leq - performs check if 'a' is less or equal than 'b'
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer
 * 
 * Returns true if 'a' represents number which is less or equal than number represented by 'b'
 * 
 * This is not supposed to be called by user. Use 'lp_uint_leq' macro instead.
 */
inline bool __lp_uint_leq(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    return __lp_uint_3way(a,a_size,b,b_size) != LP_UINT_GREATER;
}


/**
 * __lp_uint_gt - performs check if 'a' is greater than 'b'
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer
 * 
 * Returns true if 'a' represents number which is greater than number represented by 'b'
 * 
 * This is not supposed to be called by user. Use 'lp_uint_gt' macro instead.
 */
inline bool __lp_uint_gt(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    return __lp_uint_3way(a,a_size,b,b_size) == LP_UINT_GREATER;
}


/**
 * __lp_uint_geq - performs check if 'a' is greater or equal than 'b'
 * @a:       pointer on left side uint buffer
 * @a_size:  size of left side uint buffer
 * @b:       pointer on right side uint buffer
 * @b_size:  size of right side uint buffer
 * 
 * Returns true if 'a' represents number which is greater or equal than number represented by 'b'
 * 
 * This is not supposed to be called by user. Use 'lp_uint_geq' macro instead.
 */
inline bool __lp_uint_geq(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size)
{
    return __lp_uint_3way(a,a_size,b,b_size) != LP_UINT_LESS;
}


/**
 * __lp_uint_and - performs bitwise *and* operation on 'a' and 'b' storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_and' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a' or 'b'.
 */
inline bool __lp_uint_and(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return_set_errno(false,EINVAL);
    if(a == result || b == result)
        return_set_errno(false,EINVAL);
    
    size_t upper_bound = MIN(MIN(a_size,b_size),result_size);
    size_t res_i = 0;
    for(; res_i < upper_bound; ++res_i)
        result[res_i] = a[res_i] & b[res_i];
    
    for(; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    return true;
}


/**
 * __lp_uint_and_inplace - performs bitwise *and* operation on 'dest' and 'other' storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform operation with
 * @other_size: size of another uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_and_ip' macro instead.
 */
inline bool __lp_uint_and_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other, size_t other_size)
{
    if(!dest || !other)
        return_set_errno(false,EINVAL);
    
    size_t upper_bound = MIN(dest_size,other_size);
    size_t dest_i = 0;
    for(; dest_i < upper_bound; ++dest_i)
        dest[dest_i] &= other[dest_i];
    
    for(; dest_i < dest_size; ++dest_i)
        dest[dest_i] = 0;
    
    return true;
}


/**
 * __lp_uint_or - performs bitwise *or* operation on 'a' and 'b' storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_or' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a' or 'b'.
 */
inline bool __lp_uint_or(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return_set_errno(false,EINVAL);
    if(a == result || b == result)
        return_set_errno(false,EINVAL);
    
    const __lp_uint_word_t *max_term;
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

    size_t min_upper_bound = MIN(min_term_size,result_size);
    size_t res_i = 0;
    for(; res_i < min_upper_bound; ++res_i)
        result[res_i] = a[res_i] | b[res_i];
    
    size_t max_upper_bound = MIN(max_term_size,result_size);
    for(; res_i < max_upper_bound; ++res_i)
        result[res_i] = max_term[res_i];
    
    for(; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    return true;
}


/**
 * __lp_uint_or_inplace - performs bitwise *or* operation on 'dest' and 'other' storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform operation with
 * @other_size: size of another uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_or_ip' macro instead.
 */
inline bool __lp_uint_or_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other, size_t other_size)
{
    if(!dest || !other)
        return_set_errno(false,EINVAL);
    
    size_t upper_bound = MIN(dest_size,other_size);
    for(size_t i = 0; i < upper_bound; ++i)
        dest[i] |= other[i];
    
    return true;
}


/**
 * __lp_uint_xor - performs bitwise *xor* operation on 'a' and 'b' storing result in 'result'
 * @a:              pointer on left side uint buffer
 * @a_size:         size of left side uint buffer
 * @b:              pointer on right side uint buffer
 * @b_size:         size of right side uint buffer
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_xor' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a' or 'b'.
 */
inline bool __lp_uint_xor(const __lp_uint_word_t *a, size_t a_size, const __lp_uint_word_t *b, size_t b_size, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !b || !result)
        return_set_errno(false,EINVAL);
    if(a == result || b == result)
        return_set_errno(false,EINVAL);
    
    const __lp_uint_word_t *max_term;
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

    size_t min_upper_bound = MIN(min_term_size,result_size);
    size_t res_i = 0;
    for(; res_i < min_upper_bound; ++res_i)
        result[res_i] = a[res_i] ^ b[res_i];
    
    size_t max_upper_bound = MIN(max_term_size,result_size);
    for(; res_i < max_upper_bound; ++res_i)
        result[res_i] = max_term[res_i];
    
    for(; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    return true;
}


/**
 * __lp_uint_xor_inplace - performs bitwise *xor* operation on 'dest' and 'other' storing result in 'dest'
 * @dest:       pointer on destination uint buffer
 * @dest_size:  size of destination uint buffer
 * @other:      pointer on another uint buffer to perform operation with
 * @other_size: size of another uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_xor_ip' macro instead.
 */
inline bool __lp_uint_xor_inplace(__lp_uint_word_t *dest, size_t dest_size, const __lp_uint_word_t *other, size_t other_size)
{
    if(!dest || !other)
        return_set_errno(false,EINVAL);
    
    size_t upper_bound = MIN(dest_size,other_size);
    for(size_t i = 0; i < upper_bound; ++i)
        dest[i] ^= other[i];
    
    return true;
}


/**
 * __lp_uint_lshift - performs bits left shift storing result in 'result'
 * @a:              pointer on uint buffer
 * @a_size:         size of uint buffer
 * @shift:          shift size
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_lshift' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a'
 */
inline bool __lp_uint_lshift(const __lp_uint_word_t *a, size_t a_size, size_t shift, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !result)
        return_set_errno(false,EINVAL);
    if(a == result)
        return_set_errno(false,EINVAL);

    shift = MIN(result_size*__LP_UINT_BITS_PER_WORD,shift);
    size_t shift_words = shift / __LP_UINT_BITS_PER_WORD;
    size_t shift_bits = shift % __LP_UINT_BITS_PER_WORD;

    size_t res_i = 0;
    for(; res_i < shift_words; ++res_i)
        result[res_i] = 0;

    if(result_size == res_i)
        return true;

    result[res_i++] = a[0] << shift_bits;

    size_t upper_bound = MIN(result_size,a_size+shift_words);
    for(; res_i < upper_bound; ++res_i)
    {
        uint16_t shift_rem = __LP_UINT_BITS_PER_WORD-shift_bits;
        __lp_uint_word_t remainder = shift_rem == __LP_UINT_BITS_PER_WORD ? 0 : (a[res_i-shift_words-1] >> shift_rem);
        result[res_i] = (a[res_i-shift_words] << shift_bits) | remainder;
    }

    if(result_size == res_i)
        return true;
    
    uint16_t shift_rem = __LP_UINT_BITS_PER_WORD-shift_bits;
    __lp_uint_word_t remainder = shift_rem == __LP_UINT_BITS_PER_WORD ? 0 : (a[res_i-shift_words-1] >> shift_rem);
    result[res_i++] = remainder;
    
    for(; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    return true;
}


/**
 * __lp_uint_lshift_inplace - performs bits left shift storing result in 'a'
 * @a:          pointer on destination uint buffer
 * @a_size:     size of destination uint buffer
 * @shift:      shift size
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_lshift_ip' macro instead.
 */
inline bool __lp_uint_lshift_inplace(__lp_uint_word_t *a, size_t a_size, size_t shift)
{
    if(!a)
        return_set_errno(false,EINVAL);

    shift = MIN(a_size*__LP_UINT_BITS_PER_WORD,shift);
    size_t shift_words = shift / __LP_UINT_BITS_PER_WORD;
    size_t shift_bits = shift % __LP_UINT_BITS_PER_WORD;

    int64_t word_i = a_size-1;
    for(; word_i > shift_words; --word_i)
    {
        uint16_t shift_rem = __LP_UINT_BITS_PER_WORD-shift_bits;
        __lp_uint_word_t remainder = shift_rem == __LP_UINT_BITS_PER_WORD ? 0 : (a[word_i-shift_words-1] >> shift_rem);
        a[word_i] = (a[word_i-shift_words] << shift_bits) | remainder;
    }

    if(word_i == shift_words)
        a[word_i--] = a[0] << shift_bits;

    for(; word_i >= 0; --word_i)
        a[word_i] = 0;

    
    return true;
}


/**
 * __lp_uint_rshift - performs bits right shift
 * @a:              pointer on uint buffer
 * @a_size:         size of uint buffer
 * @shift:          shift size
 * @result:         pointer on result uint buffer
 * @result_size:    size of result uint buffer
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_rshift' macro instead.
 * 
 * CAUTION: 'result' can't point on the same memory region as 'a'
 */
inline bool __lp_uint_rshift(const __lp_uint_word_t *a, size_t a_size, size_t shift, __lp_uint_word_t *result, size_t result_size)
{
    if(!a || !result)
        return_set_errno(false,EINVAL);
    if(a == result)
        return_set_errno(false,EINVAL);

    shift = MIN(a_size*__LP_UINT_BITS_PER_WORD,shift);
    size_t shift_words = shift / __LP_UINT_BITS_PER_WORD;
    size_t shift_bits = shift % __LP_UINT_BITS_PER_WORD;

    size_t res_i = 0;
    size_t upper_bound = MIN(
        result_size,
        a_size < shift_words+1 ? 0 : a_size-shift_words-1);
    for(; res_i < upper_bound; ++res_i)
    {
        uint16_t shift_rem = __LP_UINT_BITS_PER_WORD-shift_bits;
        __lp_uint_word_t remainder = shift_rem == __LP_UINT_BITS_PER_WORD ? 0 : (a[res_i+shift_words+1] << shift_rem);
        result[res_i] = (a[res_i+shift_words] >> shift_bits) | remainder;
    }

    if(result_size == res_i)
        return true;
    
    if(res_i+shift_words < a_size)
    {
        result[res_i] = a[res_i+shift_words] >> shift_bits;
        ++res_i;
    }
    
    for(; res_i < result_size; ++res_i)
        result[res_i] = 0;
    
    return true;
}


/**
 * __lp_uint_rshift_inplace - performs bits right shift storing result in 'a'
 * @a:          pointer on destination uint buffer
 * @a_size:     size of destination uint buffer
 * @shift:      shift size
 * 
 * Returns true on success and false on failure.
 * 
 * This is not supposed to be called by user. Use 'lp_uint_rshift_ip' macro instead.
 */
inline bool __lp_uint_rshift_inplace(__lp_uint_word_t *a, size_t a_size, size_t shift)
{
    if(!a)
        return_set_errno(false,EINVAL);

    shift = MIN(a_size*__LP_UINT_BITS_PER_WORD,shift);
    size_t shift_words = shift / __LP_UINT_BITS_PER_WORD;
    size_t shift_bits = shift % __LP_UINT_BITS_PER_WORD;

    size_t word_i = 0;
    size_t upper_bound = a_size < shift_words+1 ? 0 : a_size-shift_words-1;
    for(; word_i < upper_bound; ++word_i)
    {
        uint16_t shift_rem = __LP_UINT_BITS_PER_WORD-shift_bits;
        __lp_uint_word_t remainder = shift_rem == __LP_UINT_BITS_PER_WORD ? 0 : (a[word_i+shift_words+1] << shift_rem);
        a[word_i] = (a[word_i+shift_words] >> shift_bits) | remainder;
    }
    
    if(word_i+shift_words < a_size)
    {
        a[word_i] = a[word_i+shift_words] >> shift_bits;
        ++word_i;
    }
    
    for(; word_i < a_size; ++word_i)
        a[word_i] = 0;
    
    return true;
}


inline bool __lp_uint_rand(__lp_uint_word_t *a, size_t a_size, size_t width_high)
{
    if(!a)
        return_set_errno(false,EINVAL);
    
    width_high = MIN(width_high,a_size*__LP_UINT_BITS_PER_WORD);

    size_t whole_words = width_high / __LP_UINT_BITS_PER_WORD;
    size_t whole_words_bytes = sizeof(__lp_uint_word_t)*whole_words;
    arc4random_buf(a,whole_words_bytes);

    size_t rem_bits = width_high % __LP_UINT_BITS_PER_WORD;

    if(rem_bits > 0)
    {
        __lp_uint_word_t rem_bits_last_bit = (__lp_uint_word_t)1 << (rem_bits-1);
        __lp_uint_word_t rem_bits_mask = (rem_bits_last_bit - 1) | rem_bits_last_bit;

        #define __ARC4_OUTPUT_WIDTH 32

        a[whole_words] = arc4random() & rem_bits_mask;
        if(rem_bits >= __ARC4_OUTPUT_WIDTH)
            a[whole_words] |= ((__lp_uint_word_t)arc4random() << __ARC4_OUTPUT_WIDTH) & rem_bits_mask;
    }
    else
        a[whole_words] = 0;

    for(size_t word_i = whole_words+1; word_i < a_size; ++word_i)
        a[word_i] = 0;

    return true;
}