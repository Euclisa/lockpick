#ifndef _LOCKPICK_BITOPS_H
#define _LOCKPICK_BITOPS_H

#include <stdint.h>


uint8_t lp_bittestandreset(uint32_t *bitmap, uint32_t bit_offset);

uint8_t lp_bittestandset(uint32_t *bitmap, uint32_t bit_offset);

#endif // _LOCKPICK_BITOPS_H