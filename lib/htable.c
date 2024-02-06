#include <lockpick/htable.h>
#include <lockpick/math.h>
#include <stdlib.h>
#include <string.h>


static inline size_t __lp_htable_occupancy_bm_size(size_t capacity)
{
    return MAX(1, capacity >> 3);
}

static inline size_t __lp_htable_capacity_mask(size_t capacity)
{
    return capacity-1;
}

static inline bool __lp_htable_get_occ_bit(const uint8_t *occupancy_bm, size_t bucket_i)
{
    size_t bm_word_i = bucket_i / LP_BITS_IN_BYTE;
    size_t bm_bit_i = bucket_i % LP_BITS_IN_BYTE;
    return (occupancy_bm[bm_word_i] >> bm_bit_i) & 0b1;
}

static inline void __lp_htable_set_occ_bit(uint8_t *occupancy_bm, size_t bucket_i, bool occupied)
{
    size_t mask_word_i = bucket_i / LP_BITS_IN_BYTE;
    size_t mask_bit_i = bucket_i % LP_BITS_IN_BYTE;
    if(occupied)
        occupancy_bm[mask_word_i] |= 0b1 << mask_bit_i;
    else
        occupancy_bm[mask_word_i] &= ~(0b1 << mask_bit_i);
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


lp_htable_t *lp_htable_create(size_t entry_size, size_t capacity, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *))
{
    affirm_nullptr(hsh,"entries hash function");
    affirm_nullptr(eq,"entries equality predicate");
    affirmf(lp_is_pow_2(capacity),"Capacity must be a power of 2");

    const size_t ht_size = sizeof(lp_htable_t);
    lp_htable_t *ht = (lp_htable_t*)malloc(ht_size);
    affirm_bad_malloc(ht,"htable structure",ht_size);

    const size_t buckets_arr_size = entry_size*capacity;
    ht->__buckets = (void*)malloc(buckets_arr_size);
    affirm_bad_malloc(ht->__buckets,"buckets array",buckets_arr_size);

    const size_t occupancy_bm_size = __lp_htable_occupancy_bm_size(capacity);
    ht->__occupancy_bm = (uint8_t*)calloc(occupancy_bm_size,sizeof(uint8_t));
    affirm_bad_malloc(ht->__occupancy_bm,"occupancy bitmap",occupancy_bm_size);

    ht->__entry_size = entry_size;
    ht->__capacity = capacity;
    ht->__hsh = hsh;
    ht->__eq = eq;
    ht->__size = 0;

    return ht;
}


void lp_htable_release(lp_htable_t *ht)
{
    affirm_nullptr(ht,"htable");
    free(ht->__buckets);
    free(ht->__occupancy_bm);
    free(ht);
}


static inline const void *__lp_htable_insert(lp_htable_t *ht, const void *entry)
{
    size_t capacity_mask = __lp_htable_capacity_mask(ht->__capacity);
    size_t bucket_i = ht->__hsh(entry) & capacity_mask;
    void *ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
    while(__lp_htable_get_occ_bit(ht->__occupancy_bm,bucket_i))
    {
        if(ht->__eq(entry,ht_entry))
            return NULL;
        bucket_i = (bucket_i + 1) & capacity_mask;
        ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
    }
    __lp_htable_set_occ_bit(ht->__occupancy_bm,bucket_i,true);
    memcpy(ht_entry,entry,ht->__entry_size);
    ++ht->__size;
    
    return ht_entry;
}


void __lp_htable_rehash(lp_htable_t *ht, size_t new_capacity)
{
    affirmf_debug(lp_is_pow_2(new_capacity),"Capacity must be a power of 2");

    void *old_buckets = ht->__buckets;
    size_t new_buckets_arr_size = ht->__entry_size*new_capacity;
    ht->__buckets = (void*)malloc(new_buckets_arr_size);
    affirm_bad_malloc(ht->__buckets,"new buckets array",new_buckets_arr_size);

    uint8_t *old_occupancy_bm = ht->__occupancy_bm;
    size_t new_occupancy_bm_size = __lp_htable_occupancy_bm_size(new_capacity);
    ht->__occupancy_bm = (uint8_t*)calloc(new_occupancy_bm_size,sizeof(uint8_t));
    affirm_bad_malloc(ht->__occupancy_bm,"new occupancy bitmap",new_occupancy_bm_size);

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
}


const void *lp_htable_insert(lp_htable_t *ht, const void *entry)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"entry to be inserted");

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
    __lp_htable_set_occ_bit(ht->__occupancy_bm,prev_bucket_i,false);
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


const void *lp_htable_find(lp_htable_t *ht, const void *entry)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"look-up entry");

    return __lp_htable_find(ht,entry);
}


bool lp_htable_remove(lp_htable_t *ht, const void *entry)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"entry to be removed");
    
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


void lp_htable_rehash(lp_htable_t *ht, size_t new_size)
{
    affirm_nullptr(ht,"htable");

    new_size = MAX(1,new_size);
    size_t new_capacity;
    if(lp_is_pow_2(new_size))
        new_capacity = new_size;
    else
        new_capacity = 1ULL << (lp_floor_log2(new_size)+2);

    __lp_htable_rehash(ht,new_capacity);
}