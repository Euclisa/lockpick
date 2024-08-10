#include <lockpick/bitset.h>
#include <lockpick/affirmf.h>
#include <lockpick/math.h>
#include <malloc.h>


static inline size_t __lp_bitset_bitmap_words_num(size_t bits_size)
{
    return lp_ceil_div_u64(bits_size,LP_BITSET_WORD_WIDTH);
}


lp_bitset_t *lp_bitset_create(size_t size)
{
    affirmf(size > 0,"Size must be greater than zero");

    size_t bits_size = sizeof(lp_bitset_t);
    lp_bitset_t *bits = (lp_bitset_t*)malloc(bits_size);
    affirm_bad_malloc(bits,"bitset",bits_size);

    size_t buffer_words_num = __lp_bitset_bitmap_words_num(size);
    bits->__bitmap = (uint32_t*)calloc(buffer_words_num,sizeof(uint32_t));
    affirm_bad_malloc(bits->__bitmap,"bitset buffer",buffer_words_num*sizeof(uint32_t));

    bits->size = size;
    bits->bits_set = 0;

    return bits;
}


void lp_bitset_release(lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    free(bits->__bitmap);
    free(bits);
}


bool lp_bitset_set(lp_bitset_t *bits, size_t index)
{
    affirm_nullptr(bits,"bitset");
    affirmf(index < bits->size,
        "Index %zd is out of range for bitset with size %zd",index,bits->size);

    bool old = (bool)lp_bittestandset(bits->__bitmap,index);

    if(!old)
        ++bits->bits_set;
    
    return old;
}


bool lp_bitset_reset(lp_bitset_t *bits, size_t index)
{
    affirm_nullptr(bits,"bitset");
    affirmf(index < bits->size,
        "Index %zd is out of range for bitset with size %zd",index,bits->size);

    bool old = (bool)lp_bittestandreset(bits->__bitmap,index);

    if(old)
        --bits->bits_set;

    return old;
}


void lp_bitset_set_all(lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    size_t bitmap_words_num = __lp_bitset_bitmap_words_num(bits->size);
    for(size_t word_i = 0; word_i < bitmap_words_num; ++word_i)
        bits->__bitmap[word_i] = (uint32_t)-1ULL;
    bits->bits_set = bits->size;
}


void lp_bitset_reset_all(lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    size_t bitmap_words_num = __lp_bitset_bitmap_words_num(bits->size);
    for(size_t word_i = 0; word_i < bitmap_words_num; ++word_i)
        bits->__bitmap[word_i] = 0;
    bits->bits_set = 0;
}


bool lp_bitset_update(lp_bitset_t *bits, size_t index, bool value)
{
    affirm_nullptr(bits,"bitset");
    affirmf(index < bits->size,
        "Index %zd is out of range for bitset with size %zd",index,bits->size);
    
    bool old;
    if(value)
    {
        old = (bool)lp_bittestandset(bits->__bitmap,index);
        if(!old)
            ++bits->bits_set;
    }
    else
    {
        old = (bool)lp_bittestandreset(bits->__bitmap,index);
        if(old)
            --bits->bits_set;
    }
    
    return old;
}


bool lp_bitset_test(const lp_bitset_t *bits, size_t index)
{
    affirm_nullptr(bits,"bitset");
    affirmf(index < bits->size,
        "Index %zd is out of range for bitset with size %zd",index,bits->size);

    return (bool)lp_bittest(bits->__bitmap,index);
}


size_t lp_bitset_count(const lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    return bits->bits_set;
}


bool lp_bitset_any(const lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    return (bool)bits->bits_set;
}


bool lp_bitset_all(const lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    return bits->bits_set == bits->size;
}


bool lp_bitset_none(const lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    return bits->bits_set == 0;
}


void lp_bitset_copy(lp_bitset_t *dest, const lp_bitset_t *src)
{
    affirm_nullptr(dest,"destination bitset");
    affirm_nullptr(src,"source bitset");

    size_t words_upper_bound = MIN(__lp_bitset_bitmap_words_num(dest->size),__lp_bitset_bitmap_words_num(src->size));
    for(size_t word_i = 0; word_i < words_upper_bound; ++word_i)
        dest->__bitmap[word_i] = src->__bitmap[word_i];
}


uint32_t *lp_bitset_extract_bitmap(lp_bitset_t *bits)
{
    affirm_nullptr(bits,"bitset");

    uint32_t *bitmap = bits->__bitmap;
    free(bits);

    return bitmap;
}
