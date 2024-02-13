#include <lockpick/sync/shtable.h>
#include <lockpick/emalloc.h>
#include <lockpick/math.h>
#include <lockpick/sync/bitops.h>
#include <lockpick/bitops.h>
#include <string.h>


enum __lp_shtable_sync_blocks
{
    __LP_SHT_REHASH,
    __LP_SHT_FIND,
    __LP_SHT_INSERT,
    __LP_SHT_INSERT_CHECK_REHASH,
    __LP_SHT_REMOVE,
    __LP_SHT_REMOVE_CHECK_REHASH,
    __LP_SHT_SIZE,

};

#define __LP_SHTABLE_SYNC_BLOCKS_NUM 7


static inline size_t __lp_shtable_capacity_mask(size_t capacity)
{
    return capacity-1;
}


static inline size_t __lp_shtable_occupancy_bm_size(size_t capacity)
{
    return MAX(1, capacity >> 5);
}


static inline bool __lp_shtable_get_occ_bit(const uint32_t *occupancy_bm, size_t bucket_i)
{
    size_t bm_word_i = bucket_i / (LP_BITS_IN_BYTE*sizeof(uint32_t));
    size_t bm_bit_i = bucket_i % (LP_BITS_IN_BYTE*sizeof(uint32_t));
    return (occupancy_bm[bm_word_i] >> bm_bit_i) & 0b1;
}


static inline bool __lp_shtable_atomic_test_set_occ_bit(uint32_t *occupancy_bm, size_t bucket_i)
{
    size_t bm_word_i = bucket_i / (LP_BITS_IN_BYTE*sizeof(uint32_t));
    size_t bm_bit_i = bucket_i % (LP_BITS_IN_BYTE*sizeof(uint32_t));
    return lp_atomic_bittestandset(&occupancy_bm[bm_word_i],bm_bit_i);
}


static inline void __lp_shtable_reset_occ_bit(uint32_t *occupancy_bm, size_t bucket_i)
{
    size_t bm_word_i = bucket_i / (LP_BITS_IN_BYTE*sizeof(uint32_t));
    size_t bm_bit_i = bucket_i % (LP_BITS_IN_BYTE*sizeof(uint32_t));
    occupancy_bm[bm_word_i] &= ~((uint32_t)0b1 << bm_bit_i);
}


lp_shtable_t *lp_shtable_create(size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *))
{
    if(!hsh || !eq)
        return_set_errno(NULL,EINVAL);

    lp_shtable_t *ht = (lp_shtable_t*)emalloc(1,sizeof(lp_shtable_t),NULL);

    ht->__buckets = (void*)emalloc(__LP_SHTABLE_INIT_CAPACITY,entry_size,NULL);

    const size_t occupancy_bm_size = __lp_shtable_occupancy_bm_size(__LP_SHTABLE_INIT_CAPACITY);
    ht->__occupancy_bm = (uint32_t*)ecalloc(occupancy_bm_size,sizeof(uint32_t),NULL);

    ht->__lgraph = lp_lock_graph_create(__LP_SHTABLE_SYNC_BLOCKS_NUM);
    // 'Rehash' block blocks everyone, but explicitly need to lock only 'find' block
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_REHASH,__LP_SHT_FIND),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_REHASH,__LP_SHT_REMOVE),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_REHASH,__LP_SHT_INSERT),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_REHASH,__LP_SHT_REHASH),false,NULL);
    
    // 'Remove' block locks everyone
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_REMOVE,__LP_SHT_FIND),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_INSERT,__LP_SHT_REMOVE),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_REMOVE,__LP_SHT_INSERT_CHECK_REHASH),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_REMOVE,__LP_SHT_REMOVE),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_REMOVE_CHECK_REHASH,__LP_SHT_INSERT_CHECK_REHASH),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_REMOVE_CHECK_REHASH,__LP_SHT_INSERT),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_REMOVE_CHECK_REHASH,__LP_SHT_REMOVE),false,NULL);

    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_INSERT,__LP_SHT_SIZE),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep_mutual(ht->__lgraph,__LP_SHT_SIZE,__LP_SHT_INSERT_CHECK_REHASH),false,NULL);

    // 'Check rehash' blocks lock theirselves
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_INSERT_CHECK_REHASH,__LP_SHT_INSERT_CHECK_REHASH),false,NULL);
    check_fatal_debug(lp_lock_graph_add_dep(ht->__lgraph,__LP_SHT_REMOVE_CHECK_REHASH,__LP_SHT_REMOVE_CHECK_REHASH),false,NULL);
    check_fatal_debug(lp_lock_graph_commit(ht->__lgraph),false,NULL);

    ht->__entry_size = entry_size;
    ht->__capacity = __LP_SHTABLE_INIT_CAPACITY;
    ht->__hsh = hsh;
    ht->__eq = eq;
    ht->__size = 0;

    return ht;
}


bool lp_shtable_release(lp_shtable_t *ht)
{
    if(!ht)
        return_set_errno(NULL,EINVAL);
    free(ht->__buckets);
    free(ht->__occupancy_bm);
    check_fatal_debug(lp_lock_graph_release(ht->__lgraph),false,false);
    free(ht);

    return true;
}


static inline void *__lp_shtable_find(lp_shtable_t *ht, const void *entry)
{
    size_t capacity_mask = __lp_shtable_capacity_mask(ht->__capacity);
    size_t bucket_i = ht->__hsh(entry) & capacity_mask;

    if(!__lp_shtable_get_occ_bit(ht->__occupancy_bm,bucket_i))
        return NULL;

    void *ht_entry = NULL;
    while(__lp_shtable_get_occ_bit(ht->__occupancy_bm,bucket_i))
    {
        ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
        if(ht->__eq(ht_entry,entry))
            return ht_entry;
        bucket_i = (bucket_i + 1) & capacity_mask;
    }

    return NULL;
}


bool lp_shtable_find(lp_shtable_t *ht, const void *entry, void *result)
{
    if(!ht || !entry)
        return_set_errno(NULL,EINVAL);

    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_FIND),false,NULL);

    const void *ht_entry = __lp_shtable_find(ht,entry);
    if(ht_entry)
        memcpy(result,ht_entry,ht->__entry_size);
    
    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_FIND),false,NULL);

    return ht_entry;
}


static inline bool __lp_shtable_insert(lp_shtable_t *ht, const void *entry)
{
    size_t capacity_mask = __lp_shtable_capacity_mask(ht->__capacity);
    size_t bucket_i = ht->__hsh(entry) & capacity_mask;
    void *ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
    while(__lp_shtable_atomic_test_set_occ_bit(ht->__occupancy_bm,bucket_i))
    {
        if(ht->__eq(entry,ht_entry))
            return false;
        bucket_i = (bucket_i + 1) & capacity_mask;
        ht_entry = ht->__buckets + bucket_i*ht->__entry_size;
    }
    memcpy(ht_entry,entry,ht->__entry_size);
    
    return true;
}


static inline size_t __lp_shtable_relaxed_capacity(size_t size)
{
    size = MAX(1,size);
    return MAX(__LP_SHTABLE_INIT_CAPACITY, 1 << (lp_ceil_log2(size)+1));
}

static inline size_t __lp_shtable_rehash_required(lp_shtable_t *ht)
{
    size_t relaxed_cap = __lp_shtable_relaxed_capacity(ht->__size);
    if(relaxed_cap > ht->__capacity || relaxed_cap < (ht->__capacity >> 1))
        return relaxed_cap;
    return 0;
}


bool __lp_shtable_rehash(lp_shtable_t *ht)
{
    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_REHASH),false,NULL);

    size_t new_capacity = __lp_shtable_rehash_required(ht);
    if(new_capacity)
    {
        void *old_buckets = ht->__buckets;
        size_t new_buckets_arr_size = ht->__entry_size*new_capacity;
        ht->__buckets = (void*)malloc(new_buckets_arr_size);

        uint32_t *old_occupancy_bm = ht->__occupancy_bm;
        size_t new_occupancy_bm_size = __lp_shtable_occupancy_bm_size(new_capacity);
        ht->__occupancy_bm = (uint32_t*)calloc(new_occupancy_bm_size,sizeof(uint32_t));

        size_t old_capacity = ht->__capacity;
        ht->__capacity = new_capacity;

        void *entry = old_buckets;
        for(size_t bucket_i = 0; bucket_i < old_capacity; ++bucket_i)
        {
            if(__lp_shtable_get_occ_bit(old_occupancy_bm,bucket_i))
                __lp_shtable_insert(ht,entry);
            entry += ht->__entry_size;
        }

        free(old_buckets);
        free(old_occupancy_bm);
    }

    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_REHASH),false,NULL);

    return new_capacity;
}


static inline void __lp_shtable_check_rehash(lp_shtable_t *ht)
{
    if(__lp_shtable_rehash_required(ht))
        __lp_shtable_rehash(ht);
}


bool lp_shtable_insert(lp_shtable_t *ht, const void *entry)
{
    if(!ht || !entry)
        return_set_errno(NULL,EINVAL);
    
    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_INSERT_CHECK_REHASH),false,NULL);

    ++ht->__size;
    __lp_shtable_check_rehash(ht);

    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_INSERT),false,NULL);
    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_INSERT_CHECK_REHASH),false,NULL);

    bool inserted = __lp_shtable_insert(ht,entry);
    if(!inserted)
        --ht->__size;

    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_INSERT),false,NULL);

    return inserted;
}


static inline void __lp_shtable_remove_found(lp_shtable_t *ht, void *ht_entry)
{
    size_t capacity_mask = __lp_shtable_capacity_mask(ht->__capacity);
    size_t bucket_i = (ht_entry-ht->__buckets) / ht->__entry_size;

    void *prev = ht_entry;
    size_t prev_bucket_i = bucket_i;
    bucket_i = (bucket_i + 1) & capacity_mask;
    while(__lp_shtable_get_occ_bit(ht->__occupancy_bm,bucket_i))
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
    __lp_shtable_reset_occ_bit(ht->__occupancy_bm,prev_bucket_i);
}


bool lp_shtable_remove(lp_shtable_t *ht, const void *entry)
{
    if(!ht || !entry)
        return_set_errno(NULL,EINVAL);
    
    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_REMOVE),false,NULL);

    void *ht_entry = __lp_shtable_find(ht,entry);
    if(ht_entry)
    {
        __lp_shtable_remove_found(ht,ht_entry);
        --ht->__size;
    }

    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_REMOVE_CHECK_REHASH),false,NULL);
    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_REMOVE),false,NULL);

    __lp_shtable_check_rehash(ht);

    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_REMOVE_CHECK_REHASH),false,NULL);

    return ht_entry;
}


size_t lp_shtable_size(lp_shtable_t *ht)
{
    if(!ht)
        return_set_errno(LP_NPOS,EINVAL);
    
    check_fatal_debug(lp_lock_graph_lock(ht->__lgraph,__LP_SHT_SIZE),false,LP_NPOS);

    size_t size = ht->__size;

    check_fatal_debug(lp_lock_graph_unlock(ht->__lgraph,__LP_SHT_SIZE),false,LP_NPOS);

    return size;
}
