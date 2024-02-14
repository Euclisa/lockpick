#ifndef _LOCKPICK_BITOPS_H
#define _LOCKPICK_BITOPS_H

#include <stdint.h>


uint8_t lp_bittestandreset(uint32_t *bitmap, uint32_t bit_offset);

uint8_t lp_bittestandset(uint32_t *bitmap, uint32_t bit_offset);

#define LP_BITS_PER_BYTE 8
#define LP_BITS_PER_HEX 4

#define lp_sizeof_bits(x) (sizeof(x)*LP_BITS_PER_BYTE)

#endif // _LOCKPICK_BITOPS_H