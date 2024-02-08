#include <lockpick/intrin.h>


inline uint8_t lp_intrin_bittestandset(uint32_t *bitmap, uint32_t bit_offset)
{
    uint8_t result;
    __asm__ volatile (
            "lock bts %[off],%[bm]\n\t"
          : [res]"=@ccc"(result), [bm]"+m"(*bitmap)
          : [off]"r"(bit_offset)
          : "memory");
    
    return result;
}
