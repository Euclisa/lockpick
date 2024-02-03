#ifndef _LOCKPICK_HTABLE_H
#define _LOCKPICK_HTABLE_H

#include <lockpick/define.h>
#include <lockpick/affirmf.h>
#include <stdint.h>


typedef struct lp_htable
{
    void *__buckets;
    uint8_t *__occupancy_bm;

    size_t (*__hsh)(void *);
    bool (*__eq)(void *, void *);

    size_t __entry_size;
    size_t __capacity;
    size_t __size;
} lp_htable_t;


lp_htable_t *lp_htable_create(size_t entry_size, size_t capacity, size_t (*hsh)(void *), bool (*eq)(void *, void *));

void *lp_htable_insert(lp_htable_t *ht, void *entry);

#endif // _LOCKPICK_HTABLE_H