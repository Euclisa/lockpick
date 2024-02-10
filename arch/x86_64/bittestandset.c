#include <lockpick/arch/x86_64/bittestandset.h>


inline uint8_t lp_bittestandset(uint32_t *bitmap, uint32_t bit_offset)
{
    uint8_t result;
    __asm__ volatile (
            "lock bts %[off],%[bm]\n\t"
          : [res]"=@ccc"(result), [bm]"+m"(*bitmap)
          : [off]"r"(bit_offset)
          : "memory");
    
    return result;
}