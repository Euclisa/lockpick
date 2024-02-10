#ifndef _LOCKPICK_HTABLE_H
#define _LOCKPICK_HTABLE_H

#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <stdint.h>
#include <stdbool.h>

// Capacity right shifts required to compute max load factor
#define __LP_HTABLE_CRSHT_LOADF_MAX 1
// Capacity right shifts required to compute min load factor
#define __LP_HTABLE_CRSHT_LOADF_MIN 3


typedef struct lp_htable
{
    void *__buckets;
    uint8_t *__occupancy_bm;

    size_t (*__hsh)(const void *);
    bool (*__eq)(const void *, const void *);

    size_t __entry_size;
    size_t __capacity;
    size_t __size;
} lp_htable_t;


lp_htable_t *lp_htable_create(size_t entry_size, size_t capacity, size_t (*hsh)(const void *), bool (*eq)(const void *, const void *));
void lp_htable_release(lp_htable_t *ht);

const void *lp_htable_insert(lp_htable_t *ht, const void *entry);

const void *lp_htable_find(lp_htable_t *ht, const void *entry);

bool lp_htable_remove(lp_htable_t *ht, const void *entry);

size_t lp_htable_size(const lp_htable_t *ht);
size_t lp_htable_capacity(const lp_htable_t *ht);

void lp_htable_rehash(lp_htable_t *ht, size_t new_size);

#endif // _LOCKPICK_HTABLE_H