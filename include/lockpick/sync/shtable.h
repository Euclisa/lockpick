#ifndef _LOCKPICK_SYNC_SHTABLE_H
#define _LOCKPICK_SYNC_SHTABLE_H

#include <lockpick/define.h>
#include <lockpick/sync/lock_graph.h>
#include <stdint.h>
#include <stdbool.h>

// Capacity right shifts required to compute max load factor
#define __LP_SHTABLE_CRSHT_LOADF_MAX 1
// Capacity right shifts required to compute min load factor
#define __LP_SHTABLE_CRSHT_LOADF_MIN 3

#define __LP_SHTABLE_INIT_CAPACITY 16


typedef struct lp_shtable
{
    void *__buckets;
    uint32_t *__occupancy_bm;

    size_t (*__hsh)(const void *);
    bool (*__eq)(const void *, const void *);

    size_t __entry_size;
    size_t __capacity;
    _Atomic size_t __size;

    lp_lock_graph_t *__lgraph;
} lp_shtable_t;


lp_shtable_t *lp_shtable_create(size_t entry_size, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *));
bool lp_shtable_release(lp_shtable_t *ht);

bool lp_shtable_insert(lp_shtable_t *ht, const void *entry);

bool lp_shtable_remove(lp_shtable_t *ht, const void *entry);

bool lp_shtable_find(lp_shtable_t *ht, const void *entry, void *result);

size_t lp_shtable_size(lp_shtable_t *ht);

#endif // _LOCKPICK_SYNC_SHTABLE_H