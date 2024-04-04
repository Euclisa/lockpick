#ifndef _LOCKPICK_BITSET_H
#define _LOCKPICK_BITSET_H

#include <lockpick/define.h>
#include <lockpick/bits.h>
#include <stdint.h>
#include <stdbool.h>

#define LP_BITSET_WORD_WIDTH lp_sizeof_bits(uint32_t)


typedef struct lp_bitset
{
    uint32_t *__bitmap;
    size_t size;
    size_t bits_set;
} lp_bitset_t;


lp_bitset_t *lp_bitset_create(size_t size);

void lp_bitset_release(lp_bitset_t *bits);

bool lp_bitset_set(lp_bitset_t *bits, size_t index);
bool lp_bitset_reset(lp_bitset_t *bits, size_t index);

void lp_bitset_set_all(lp_bitset_t *bits);
void lp_bitset_reset_all(lp_bitset_t *bits);

bool lp_bitset_update(lp_bitset_t *bits, size_t index, bool value);

bool lp_bitset_test(const lp_bitset_t *bits, size_t index);

size_t lp_bitset_count(const lp_bitset_t *bits);

bool lp_bitset_any(const lp_bitset_t *bits);
bool lp_bitset_all(const lp_bitset_t *bits);
bool lp_bitset_none(const lp_bitset_t *bits);

void lp_bitset_copy(lp_bitset_t *dest, const lp_bitset_t *src);

uint32_t *lp_bitset_extract_bitmap(lp_bitset_t *bits);

#endif // _LOCKPICK_BITSET_H