#include <lockpick/htable.h>
#include <lockpick/math.h>
#include <lockpick/bits.h>
#include <lockpick/emalloc.h>
#include <stdlib.h>
#include <string.h>


static inline size_t __lp_htable_opt_capacity(size_t elements_num)
{
    return 1ULL << (lp_ceil_log2(elements_num)+__LP_HTABLE_CRSHT_LOADF_MAX);;
}


static inline size_t __lp_htable_occupancy_bm_size(size_t capacity)
{
    return MAX(1, capacity >> 3);
}

static inline size_t __lp_htable_capacity_mask(size_t capacity)
{
    return capacity-1;
}

static inline bool __lp_htable_get_occ_bit(const uint32_t *occupancy_bm, size_t bucket_i)
{
    const size_t bits_in_occ_word = lp_sizeof_bits(uint32_t);
    size_t bm_word_i = lp_div_pow_2(bucket_i,bits_in_occ_word);
    size_t bm_bit_i = lp_mod_pow_2(bucket_i,bits_in_occ_word);
    return (occupancy_bm[bm_word_i] >> bm_bit_i) & 0b1;
}

static inline bool __lp_htable_test_set_occ_bit(uint32_t *occupancy_bm, size_t bucket_i)
{
    const size_t bits_in_occ_word = lp_sizeof_bits(uint32_t);
    size_t bm_word_i = lp_div_pow_2(bucket_i,bits_in_occ_word);
    size_t bm_bit_i = lp_mod_pow_2(bucket_i,bits_in_occ_word);
    return lp_bittestandset(&occupancy_bm[bm_word_i],bm_bit_i);
}

static inline void __lp_htable_reset_occ_bit(uint32_t *occupancy_bm, size_t bucket_i)
{
    const size_t bits_in_occ_word = lp_sizeof_bits(uint32_t);
    size_t bm_word_i = lp_div_pow_2(bucket_i,bits_in_occ_word);
    size_t bm_bit_i = lp_mod_pow_2(bucket_i,bits_in_occ_word);
    occupancy_bm[bm_word_i] &= ~(1U << bm_bit_i);
}

static inline bool __lp_htable_is_overloaded(size_t size, size_t capacity)
{
    size_t threshold_size = capacity >> __LP_HTABLE_CRSHT_LOADF_MAX;
    return size > threshold_size;
}

static inline bool __lp_htable_is_sparse(size_t size, size_t capacity)
{
    size_t threshold_size = capacity >> __LP_HTABLE_CRSHT_LOADF_MIN;
    return size < threshold_size;
}


lp_htable_t *lp_htable_create(size_t capacity, size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *))
{
    affirm_nullptr(hsh,"entry hash function");
    affirm_nullptr(eq,"entry equality predicate");
    affirmf(lp_is_pow_2(capacity),"Capacity must be a power of 2, but got: %zd",capacity);

    size_t ht_size = sizeof(lp_htable_t);
    lp_htable_t *ht = (lp_htable_t*)malloc(ht_size);
    affirm_bad_malloc(ht,"htable",ht_size);

    size_t buckets_size = capacity*entry_size;
    ht->__buckets = malloc(capacity*entry_size);
    affirm_bad_malloc(ht->__buckets,"buckets buffer",buckets_size);

    const size_t occupancy_bm_size = __lp_htable_occupancy_bm_size(capacity);
    ht->__occupancy_bm = (uint32_t*)calloc(occupancy_bm_size,sizeof(uint32_t));
    affirm_bad_malloc(ht->__occupancy_bm,"occupancy bitmap",occupancy_bm_size*sizeof(uint32_t));

    ht->__entry_size = entry_size;
    ht->__capacity = capacity;
    ht->__hsh = hsh;
    ht->__eq = eq;
    ht->__size = 0;

    return ht;
}


lp_htable_t *lp_htable_create_el_num(size_t elements_num, size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *))
{
    affirmf(elements_num > 0,"Number of elements in htable must be greater than 0");

    size_t capacity = __lp_htable_opt_capacity(elements_num);
    return lp_htable_create(capacity,entry_size,hsh,eq);
}


void lp_htable_release(lp_htable_t *ht)
{
    affirm_nullptr(ht,"htable");

    free(ht->__buckets);
    free(ht->__occupancy_bm);
    free(ht);
}


static inline bool __lp_htable_insert(lp_htable_t *ht, const void *entry)
{
    size_t capacity_mask = __lp_htable_capacity_mask(ht->__capacity);
    size_t bucket_i = ht->__hsh(entry) & capacity_mask;
    void *ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
    while(__lp_htable_test_set_occ_bit(ht->__occupancy_bm,bucket_i))
    {
        if(ht->__eq(entry,ht_entry))
            return false;
        bucket_i = (bucket_i + 1) & capacity_mask;
        ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
    }
    memcpy(ht_entry,entry,ht->__entry_size);
    ++ht->__size;
    
    return true;
}


bool __lp_htable_rehash(lp_htable_t *ht, size_t new_capacity)
{
    void *old_buckets = ht->__buckets;
    size_t buckets_size = new_capacity*ht->__entry_size;
    ht->__buckets = malloc(buckets_size);
    affirm_bad_malloc(buckets_size,"buckets buffer",buckets_size);

    uint32_t *old_occupancy_bm = ht->__occupancy_bm;
    size_t new_occupancy_bm_size = __lp_htable_occupancy_bm_size(new_capacity);
    ht->__occupancy_bm = (uint32_t*)ecalloc(new_occupancy_bm_size,sizeof(uint32_t),false);

    size_t old_capacity = ht->__capacity;
    ht->__capacity = new_capacity;

    ht->__size = 0;

    void *entry = old_buckets;
    for(size_t bucket_i = 0; bucket_i < old_capacity; ++bucket_i)
    {
        if(__lp_htable_get_occ_bit(old_occupancy_bm,bucket_i))
            __lp_htable_insert(ht,entry);
        entry += ht->__entry_size;
    }

    free(old_buckets);
    free(old_occupancy_bm);

    return true;
}


bool lp_htable_insert(lp_htable_t *ht, const void *entry)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"entry");

    if(__lp_htable_is_overloaded(ht->__size+1,ht->__capacity))
        __lp_htable_rehash(ht,ht->__capacity << 1);
    
    return __lp_htable_insert(ht,entry);
}


static inline void __lp_htable_remove_found(lp_htable_t *ht, void *ht_entry)
{
    size_t capacity_mask = __lp_htable_capacity_mask(ht->__capacity);
    size_t bucket_i = (ht_entry-ht->__buckets) / ht->__entry_size;

    void *prev = ht_entry;
    size_t prev_bucket_i = bucket_i;
    bucket_i = (bucket_i + 1) & capacity_mask;
    while(__lp_htable_get_occ_bit(ht->__occupancy_bm,bucket_i))
    {
        void *curr = ht->__buckets + bucket_i*ht->__entry_size;
        size_t curr_native_bucket_i = ht->__hsh(curr) & capacity_mask;

        /*
            Need to determine if native bucket has lower index than 'prev_bucket_i'
            considering we count indices modulo capacity
        */
        bool replace =
            // If 'prev_bucket_i' and 'bucket_i' are in one cycle 
            ((prev_bucket_i < bucket_i) && (curr_native_bucket_i <= prev_bucket_i || curr_native_bucket_i > bucket_i)) ||
            // If in different cycles
            ((prev_bucket_i > bucket_i) && (curr_native_bucket_i <= prev_bucket_i && curr_native_bucket_i > bucket_i));

        if(replace)
        {
            memcpy(prev,curr,ht->__entry_size);
            prev = curr;
            prev_bucket_i = bucket_i;
        }
        bucket_i = (bucket_i + 1) & capacity_mask;
    }
    __lp_htable_reset_occ_bit(ht->__occupancy_bm,prev_bucket_i);
}


static inline void *__lp_htable_find(lp_htable_t *ht, const void *entry)
{
    size_t capacity_mask = __lp_htable_capacity_mask(ht->__capacity);
    size_t bucket_i = ht->__hsh(entry) & capacity_mask;

    if(!__lp_htable_get_occ_bit(ht->__occupancy_bm,bucket_i))
        return NULL;

    void *ht_entry = NULL;
    while(__lp_htable_get_occ_bit(ht->__occupancy_bm,bucket_i))
    {
        ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
        if(ht->__eq(ht_entry,entry))
            return ht_entry;
        bucket_i = (bucket_i + 1) & capacity_mask;
    }

    return NULL;
}


bool lp_htable_find(lp_htable_t *ht, const void *entry, void *result)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"entry");

    void *found = __lp_htable_find(ht,entry);
    if(found && result)
        memcpy(result,found,ht->__entry_size);

    return found;
}


bool lp_htable_remove(lp_htable_t *ht, const void *entry)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"entry");
    
    if(ht->__size == 0)
        return false;
    
    if(__lp_htable_is_sparse(ht->__size-1,ht->__capacity))
        __lp_htable_rehash(ht,ht->__capacity >> 1);
    
    void *ht_entry = __lp_htable_find(ht,entry);
    if(!ht_entry)
        return false;

    __lp_htable_remove_found(ht,ht_entry);
    --ht->__size;

    return true;
}


inline size_t lp_htable_size(const lp_htable_t *ht)
{
    return ht->__size;
}


inline size_t lp_htable_capacity(const lp_htable_t *ht)
{
    return ht->__capacity;
}


bool lp_htable_rehash(lp_htable_t *ht, size_t new_size)
{
    affirm_nullptr(ht,"htable");
    affirmf(new_size,"New size must be greater than 0");

    size_t new_capacity = __lp_htable_opt_capacity(new_size);

    return __lp_htable_rehash(ht,new_capacity);
}