#include <lockpick/htable.h>
#include <lockpick/math.h>
#include <stdlib.h>


lp_htable_t *lp_htable_create(size_t entry_size, size_t capacity, size_t (*hsh)(void *), bool (*eq)(void *, void *))
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

    const size_t occupancy_bm_size = MAX(1,capacity/8);
    ht->__occupancy_bm = (uint8_t*)malloc(occupancy_bm_size);
    affirm_bad_malloc(ht->__occupancy_bm,"occupancy bitmap",occupancy_bm_size);

    ht->__entry_size = entry_size;
    ht->__capacity = capacity;
    ht->__hsh = hsh;
    ht->__eq = eq;
    ht->__size = 0;
}


static inline size_t __lp_htable_capacity_mask(size_t capacity)
{
    return capacity-1;
}


static inline bool __lp_htable_bucket_occupied(const lp_htable_t *ht, size_t bucket_i)
{
    size_t mask_word_i = bucket_i / LP_BITS_IN_BYTE;
    size_t mask_bit_i = bucket_i % LP_BITS_IN_BYTE;
    return (ht->__occupancy_bm[mask_word_i] >> mask_bit_i) & 0b1;
}


void __lp_htable_rehash(lp_htable_t *ht, size_t new_capacity)
{
    affirmf_debug(lp_is_pow_2(new_capacity),"Capacity must be a power of 2");

    size_t new_buckets_arr_size = ht->__entry_size*new_capacity;
    void *new_buckets = (void*)malloc(new_buckets_arr_size);
    affirm_bad_malloc(new_buckets,"new buckets array",new_buckets_arr_size);

    

    size_t new_capacity_mask = __lp_htable_capacity_mask(new_capacity);

    for(size_t bucket_i = 0; bucket_i < ht->__capacity; ++bucket_i)
    {
        if(!__lp_htable_bucket_occupied(ht,bucket_i))
            continue;
        void *entry_ptr = ht->__buckets+bucket_i*ht->__entry_size;
        size_t new_bucket_i = ht->__hsh(entry_ptr) & new_capacity_mask;

    }
}


void *lp_htable_insert(lp_htable_t *ht, void *entry)
{
    affirm_nullptr(ht,"htable");
    affirm_nullptr(entry,"entry to be inserted");

    size_t bucket_i = ht->__hsh(entry) & __lp_htable_capacity_mask(ht->__capacity);
    while(bucket_i < ht->__capacity && __lp_htable_bucket_occupied(ht,bucket_i))
        ++bucket_i;
    
    if(bucket_i == ht->__capacity)
    {
        
    }
}