#ifndef _LOCKPICK_ARCH_X86_64_BITTESTANDRESET_H
#define _LOCKPICK_ARCH_X86_64_BITTESTANDRESET_H

#include <stdint.h>


uint8_t lp_bittestandreset(uint32_t *bitmap, uint32_t bit_offset);

#endif // _LOCKPICK_ARCH_X86_64_BITTESTANDRESET_H