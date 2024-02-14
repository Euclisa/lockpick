#ifndef _LOCKPICK_SYNC_VISIT_TABLE_H
#define _LOCKPICK_SYNC_VISIT_TABLE_H

#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <lockpick/errno.h>
#include <lockpick/sync/spinlock_bitset.h>
#include <stdint.h>
#include <stdbool.h>

// Capacity right shifts required to compute max load factor
#define __LP_VISIT_TABLE_CRSHT_LOADF_MAX 1


typedef struct lp_visit_table
{
    void *__buckets;
    uint32_t *__occupancy_bm;

    size_t (*__hsh)(const void *);
    bool (*__eq)(const void *, const void *);

    size_t __entry_size;
    size_t __capacity;
    size_t __capacity_mask;

    lp_spinlock_bitset_t *__spins;
} lp_visit_table_t;


lp_visit_table_t *lp_visit_table_create(size_t capacity, size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *));
lp_visit_table_t *lp_visit_table_create_max_el(size_t max_elements, size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *));

bool lp_visit_table_release(lp_visit_table_t *vt);

bool lp_visit_table_insert(lp_visit_table_t *vt, const void *entry);

bool lp_visit_table_find(lp_visit_table_t *vt, const void *entry, void *result);

#endif // _LOCKPICK_SYNC_VISIT_TABLE_H