#ifndef _LOCKPICK_SYNC_SPINLOCK_BITSET_H
#define _LOCKPICK_SYNC_SPINLOCK_BITSET_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ENLCKD 1203 


typedef struct lp_spinlock_bitset
{
    uint32_t *__bitset;
    size_t __locks_num;
} lp_spinlock_bitset_t;


lp_spinlock_bitset_t *lp_spinlock_bitset_create(size_t n);
void lp_spinlock_bitset_release(lp_spinlock_bitset_t *spins);

bool lp_spinlock_bitset_lock(lp_spinlock_bitset_t *spins, size_t lock_i);
bool lp_spinlock_bitset_unlock(lp_spinlock_bitset_t *spins, size_t lock_i);

#endif // _LOCKPICK_SYNC_SPINLOCK_BITSET_H