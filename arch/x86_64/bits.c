#include <lockpick/bits.h>


inline uint8_t lp_bittestandreset(uint32_t *bitmap, uint32_t bit_offset)
{
    uint8_t result;
    __asm__ volatile (
            "btr %[off],%[bm]\n\t"
          : [res]"=@ccc"(result), [bm]"+m"(*bitmap)
          : [off]"r"(bit_offset)
          : "memory");
    
    return result;
}


inline uint8_t lp_bittestandset(uint32_t *bitmap, uint32_t bit_offset)
{
    uint8_t result;
    __asm__ volatile (
            "bts %[off],%[bm]\n\t"
          : [res]"=@ccc"(result), [bm]"+m"(*bitmap)
          : [off]"r"(bit_offset)
          : "memory");
    
    return result;
}