#ifndef _LOCKPICK_SYNC_BITS_H
#define _LOCKPICK_SYNC_BITS_H

#include <lockpick/bits.h>
#include <stdint.h>


uint8_t lp_atomic_bittestandreset(uint32_t *bitmap, uint32_t bit_offset);

uint8_t lp_atomic_bittestandset(uint32_t *bitmap, uint32_t bit_offset);

#endif // _LOCKPICK_SYNC_BITS_H