#include <lockpick/sync/visit_table.h>
#include <lockpick/sync/bits.h>
#include <lockpick/math.h>
#include <lockpick/emalloc.h>
#include <lockpick/affirmf.h>
#include <stdlib.h>
#include <string.h>


static inline size_t __lp_visit_table_occupancy_bm_size(size_t capacity)
{
    return MAX(1, capacity >> lp_floor_log2(sizeof(uint32_t)));
}

static inline bool __lp_htable_get_occ_bit(const uint32_t *occupancy_bm, size_t bucket_i)
{
    const size_t bits_in_occ_word = lp_sizeof_bits(uint32_t);
    size_t bm_word_i = lp_div_pow_2(bucket_i,bits_in_occ_word);
    size_t bm_bit_i = lp_mod_pow_2(bucket_i,bits_in_occ_word);
    return (occupancy_bm[bm_word_i] >> bm_bit_i) & 0b1;
}

static inline bool __lp_visit_table_test_set_occ_bit(uint32_t *occupancy_bm, size_t bucket_i)
{
    const size_t bits_in_occ_word = lp_sizeof_bits(uint32_t);
    size_t bm_word_i = lp_div_pow_2(bucket_i,bits_in_occ_word);
    size_t bm_bit_i = lp_mod_pow_2(bucket_i,bits_in_occ_word);
    return lp_atomic_bittestandset(&occupancy_bm[bm_word_i],bm_bit_i);
}


lp_visit_table_t *lp_visit_table_create(size_t capacity, size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *))
{
    affirm_nullptr(hsh,"entry hash function");
    affirm_nullptr(eq,"entry hash function");
    affirmf(lp_is_pow_2(capacity),"Capacity must be a power of 2");

    const size_t vt_size = sizeof(lp_visit_table_t);
    lp_visit_table_t *vt = (lp_visit_table_t*)malloc(vt_size);
    affirm_bad_malloc(vt,"visit table",vt_size);

    const size_t buckets_size = capacity*entry_size;
    vt->__buckets = (void*)malloc(buckets_size);
    affirm_bad_malloc(vt->__buckets,"buckets buffer",buckets_size);

    const size_t occupancy_bm_size = __lp_visit_table_occupancy_bm_size(capacity);
    vt->__occupancy_bm = (uint32_t*)calloc(occupancy_bm_size,sizeof(uint32_t));
    affirm_bad_malloc(vt->__occupancy_bm,"occupancy bitmap",occupancy_bm_size*sizeof(uint32_t));

    vt->__spins = lp_spinlock_bitset_create(capacity);

    vt->__entry_size = entry_size;
    vt->__capacity = capacity;
    vt->__capacity_mask = capacity-1;
    vt->__hsh = hsh;
    vt->__eq = eq;

    return vt;
}


lp_visit_table_t *lp_visit_table_create_max_el(size_t max_elements, size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *))
{
    affirmf(max_elements > 0,"Max elements number must be greater than 0");
    
    size_t capacity = 1ULL << (lp_ceil_log2(max_elements)+1);
    return lp_visit_table_create(capacity,entry_size,hsh,eq);
}


void lp_visit_table_release(lp_visit_table_t *vt)
{
    affirm_nullptr(vt,"visit table");

    lp_spinlock_bitset_release(vt->__spins);
    free(vt->__buckets);
    free(vt->__occupancy_bm);
    free(vt);
}


bool lp_visit_table_insert(lp_visit_table_t *vt, const void *entry)
{
    affirm_nullptr(vt,"visit table");
    affirm_nullptr(entry,"entry");

    size_t bucket_i = vt->__hsh(entry) & vt->__capacity_mask;
    size_t native_bucket_i = bucket_i;
    while(true)
    {
        void *ht_entry = vt->__buckets + bucket_i*vt->__entry_size;
        bool occupied = true;
        bool duplicate = false;

        lp_spinlock_bitset_lock(vt->__spins,bucket_i);
        occupied = __lp_visit_table_test_set_occ_bit(vt->__occupancy_bm,bucket_i);
        if(occupied)
            duplicate = vt->__eq(entry,ht_entry);
        else
            memcpy(ht_entry,entry,vt->__entry_size);
        lp_spinlock_bitset_unlock(vt->__spins,bucket_i);

        return_set_errno_on(duplicate,false,EDUP);
        return_on(!occupied,true);

        bucket_i = (bucket_i + 1) & vt->__capacity_mask;
        affirmf(bucket_i != native_bucket_i,"Visit table capacity %zd exceeded",vt->__capacity);
    }
}


bool lp_visit_table_find(lp_visit_table_t *vt, const void *entry, void *result)
{
    affirm_nullptr(vt,"visit table");
    affirm_nullptr(entry,"entry");

    size_t bucket_i = vt->__hsh(entry) & vt->__capacity_mask;
    size_t native_bucket_i = bucket_i;
    while(true)
    {
        void *ht_entry = vt->__buckets + bucket_i*vt->__entry_size;
        bool occupied = true;
        bool found = false;

        lp_spinlock_bitset_lock(vt->__spins,bucket_i);
        occupied = __lp_htable_get_occ_bit(vt->__occupancy_bm,bucket_i);
        if(occupied && (found = vt->__eq(entry,ht_entry)) && result)
            memcpy(result,ht_entry,vt->__entry_size);
        lp_spinlock_bitset_unlock(vt->__spins,bucket_i);

        return_on(!occupied,false);
        return_on(found,true);

        bucket_i = (bucket_i + 1) & vt->__capacity_mask;
        return_on(bucket_i == native_bucket_i,false);
    }
}
